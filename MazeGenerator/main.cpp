// by huyang

#define SDL_MAIN_HANDLED  
#include "SDL.h"
#include "SDL_image.h"

#include <random>
#include <vector>
#include <list>

#include "Coord2D.h"

void NextAngle(std::vector<FCoord2D>& InCoord)
{
	for (auto& coord : InCoord)
	{
		if (coord.X == 0 && coord.Y == -1) { coord.X = 1; coord.Y = 0; }

		else if (coord.X == 1 && coord.Y == 0) { coord.X = 0; coord.Y = 1; }

		else if (coord.X == 0 && coord.Y == 1) { coord.X = -1; coord.Y = 0; }

		else if (coord.X == -1 && coord.Y == 0) { coord.X = 0; coord.Y = -1; }
	}
}

void NextAngle(FCoord2D& InCoord)
{
	if (InCoord.X == 0 && InCoord.Y == -1) { InCoord.X = 1; InCoord.Y = 0; }

	else if (InCoord.X == 1 && InCoord.Y == 0) { InCoord.X = 0; InCoord.Y = 1; }

	else if (InCoord.X == 0 && InCoord.Y == 1) { InCoord.X = -1; InCoord.Y = 0; }

	else if (InCoord.X == -1 && InCoord.Y == 0) { InCoord.X = 0; InCoord.Y = -1; }
}

float SnapAngle(std::vector<FCoord2D> InSource, const std::vector<FCoord2D>& InSnapTo)
{
	if (InSource.empty() || InSource.size() > 3) return 0.f;

	if (InSnapTo.size() != InSource.size()) return 0.f;

	for (auto angle : { 0.f,90.f,180.f,270.f })
	{
		bool equal = true;
		for (size_t index = 0; index < InSource.size(); ++index)
		{
			if (InSnapTo.at(index) == InSource.at(index)) {}

			else
			{
				equal = false;
				break;
			}
		}

		if (equal)
			return angle;

		NextAngle(InSource);
	}

	return 0.f;
}

struct Rect
{
	int X;
	int Y;
	int W;
	int H;
};

bool ValueInRange(int Value, int Min, int Max)
{
	return (Value >= Min) && (Value <= Max);
}

bool Overlap(Rect A, Rect B)
{
	bool overlapX = ValueInRange(A.X, B.X, B.X + B.W) || ValueInRange(B.X, A.X, A.X + A.W);

	bool overlapY = ValueInRange(A.Y, B.Y, B.Y + B.H) || ValueInRange(B.Y, A.Y, A.Y + A.H);

	return overlapX && overlapY;
}

struct Branch
{
	Branch() : leader({ 0, 0 }), follower({ 0, 0 }) { };

	Branch(const FCoord2D& InLeader, const FCoord2D& InFollower) : leader(InLeader), follower(InFollower) { };

	FCoord2D leader;

	FCoord2D follower;
};

struct Element
{
	Element() : used(false), corner(false), around(0), angle(0.f) {};

	bool used;
	bool corner;
	int around;
	// int type;
	float angle;

	void Reset()
	{
		used = false;
		corner = false;
		// type = 0;
		around = 0;
		angle = 0.f;
	}
};

class Dungeon
{
public:
	Dungeon() : border(0, 0), scene(nullptr), random(0) {}
	~Dungeon() { delete scene; }

public:
	bool Reset(const FCoord2D&, const unsigned int);
	void Draw(SDL_Renderer*, SDL_Texture*, SDL_Texture*, SDL_Texture*, const int);

private:
	int ToIndex(const FCoord2D& coord)
	{
		return coord.Y * border.X + coord.X;
	}

	FCoord2D ToCoord(const int InIndex)
	{
		return FCoord2D(InIndex % border.X, InIndex / border.X);
	}

	Element* GetElement(const FCoord2D& InCoord)
	{
		if (Valid(InCoord))
			return &scene->at(ToIndex(InCoord));
		return nullptr;
	}

	bool Valid(const FCoord2D& InCoord)
	{
		if (InCoord.X < 0 || InCoord.Y < 0 || InCoord.X >= border.X || InCoord.Y >= border.Y)
			return false;

		try { scene->at(ToIndex(InCoord)); }
		catch (...) { return false; }
		return true;
	}

	bool Usable(const FCoord2D& InCoord)
	{
		if (!Valid(InCoord)) 
			return false;

		if (InCoord.X % 2 == 0 || InCoord.Y % 2 == 0) 
			return false;

		auto element = GetElement(InCoord);
		if (element == nullptr) 
			return false;

		return !element->used;
	}

	bool Depth(FCoord2D& OutCurrent, std::list<FCoord2D>& OutCourse)
	{
		std::vector<Branch> branchs;

		for (auto offset : { FCoord2D(0, -1),FCoord2D(0, 1),FCoord2D(-1, 0),FCoord2D(1, 0) })
		{
			auto&& leader = offset * 2 + OutCurrent;

			if (Usable(leader))
			{
				branchs.push_back(Branch(leader, offset + OutCurrent));
			}
		}

		if (branchs.empty())
		{
			auto current = GetElement(OutCurrent);

			if (current)
			{
				current->used = true;
			}

			return false;
		}

		std::uniform_int_distribution<int> uniform(0, branchs.size() - 1);

		auto branch = branchs.at(uniform(random));
		auto current = GetElement(OutCurrent);
		auto follower = GetElement(branch.follower);

		if (current && follower)
		{
			current->used = true;
			follower->used = true;
		}

		OutCourse.push_back(OutCurrent);
		OutCurrent = branch.leader;

		Depth(OutCurrent, OutCourse);

		return true;
	}

	bool Generate(const FCoord2D& start)
	{
		if (!Usable(start))
			return false;

		FCoord2D current(start);
		std::list<FCoord2D> course;
		Depth(current, course);

		while (!course.empty())
		{
			current = course.back();
			course.pop_back();

			Depth(current, course);
		}

		return true;
	}

	void BuildElement()
	{
		for (int y = 0; y < border.Y; ++y)
		{
			for (int x = 0; x < border.X; ++x)
			{
				FCoord2D coord(x, y);

				auto current = GetElement(coord);
				if (current == nullptr) // Valid check
					return;

				if (!current->used) // Skip unused element
					continue;

				// 确保 '遍历周围元素的顺序' 与 '元素图形连接点的顺序' 一致
				std::vector<FCoord2D> OFFSETORDER = { FCoord2D(0, -1),FCoord2D(1, 0),FCoord2D(0, 1),FCoord2D(-1, 0) };
				for (auto count : { 0,1,2,3 })
				{
					auto element = GetElement(OFFSETORDER.at(count) + coord);
					if (element == nullptr)
					{
						break;
					}

					if (element->used == current->used) {} // 从第一个不相等的周围元素开始

					else
					{
						while (count > 0)
						{
							--count;
							NextAngle(OFFSETORDER);
						}
						break;
					}
				}

				// 使用上一步确定的 '遍历周围元素的顺序' 进行遍历，并得到 '相邻元素数量(number)' 和 '相邻边(around)'
				int number = 0;
				std::vector<FCoord2D> around;
				for (auto& offset : OFFSETORDER)
				{
					auto element = GetElement(offset + coord);
					if (element == nullptr)
					{
						around.clear();
						number = 0;
						break;
					}
					if (element->used == current->used)
					{
						around.push_back(offset);
						++number;
					}
				}

				// 用得到的 '相邻元素数量' 和 '相邻边' 来确定元素 'type' 和 'angle'
				if (number == 0) {}
				else if (number == 1) // Road End
				{
					current->around = 1;
					current->angle = SnapAngle({ FCoord2D(1, 0) }, around);
				}
				else if (number == 2)
				{
					if (around.front() + around.back() == FCoord2D(0, 0)) // Line
					{
						current->around = 2;
						current->angle = SnapAngle({ FCoord2D(1, 0), FCoord2D(-1, 0) }, around);
					}
					else // Corner
					{
						current->corner = true;
						current->around = 2;
						current->angle = SnapAngle({ FCoord2D(1, 0), FCoord2D(0, 1) }, around);
					}
				}
				else if (number == 3) // 3 Branch
				{
					current->around = 3;
					current->angle = SnapAngle({ FCoord2D(1, 0), FCoord2D(0, 1), FCoord2D(-1, 0) }, around);
				}
				else if (number == 4) { current->around = 4; } // 4 Branch
				else {} // Other
			}
		}
	}

	std::vector<Rect> CreateRoom(const int InSize)
	{
		std::vector<Rect> result;

		if (InSize < 3 || InSize > border.X || InSize > border.Y)
			return result;

		std::uniform_int_distribution<int> X(InSize, border.X);
		std::uniform_int_distribution<int> Y(InSize, border.Y);
		std::uniform_int_distribution<int> size(InSize / 2, InSize);

		int count = 100;
		while (--count)
		{
			Rect room;
			room.W = size(random) / 2 * 2;
			room.H = size(random) / 2 * 2;
			room.X = (X(random) - room.W) / 2 * 2;
			room.Y = (Y(random) - room.H) / 2 * 2;

			bool overlap = false;
			for (auto& other : result)
			{
				overlap = Overlap(other, room);
				if (overlap)
					break;
			}

			if (!overlap)
				result.push_back(room);
		}

		return result;
	}

	FCoord2D border;
	std::vector<Element>* scene;
	std::default_random_engine random;
};


bool Dungeon::Reset(const FCoord2D& InBorder, const unsigned int InSeed = 0)
{
	border.X = InBorder.X < 3 ? 3 : InBorder.X;
	border.Y = InBorder.Y < 3 ? 3 : InBorder.Y;

	if (border.X % 2 == 0) ++border.X;
	if (border.Y % 2 == 0) ++border.Y;

	delete scene;

	scene = new (std::nothrow) std::vector<Element>(border.X * border.Y);
	if (!scene)
	{
		border.X = 0;
		border.Y = 0;

		return false;
	}

	random.seed(InSeed);

	auto rooms = CreateRoom(12);
	std::vector<Element*> space;
	for (int y = 0; y < border.Y; ++y)
	{
		for (int x = 0; x < border.X; ++x)
		{
			auto current = GetElement(FCoord2D(x, y));

			for (auto& room : rooms)
			{
				if (x > room.X && y > room.Y && x < room.X + room.W && y < room.Y + room.H)
				{
					current->used = true;
					// space.push_back(current);
				}

				if (x == room.X && y == room.Y + room.H / 2) // Y
				{
					current->used = true;
				}
				else if (y == room.Y && x == room.X + room.W / 2) // X
				{
					current->used = true;
				}
				else if (x == room.X + room.W && y == room.Y + room.H / 2)
				{
					current->used = true;
				}
				else if (y == room.Y + room.H && x == room.X + room.W / 2)
				{
					current->used = true;
				}
			}
		}
	}

	Generate(FCoord2D(1, 1));

	/*
	for (size_t i = 0; i < 16; i++)
	{
		// Slack();
	}
	*/

	BuildElement();

	return true;
}

void Dungeon::Draw(SDL_Renderer* renderer, SDL_Texture* None, SDL_Texture* Road, SDL_Texture* Space, const int size = 16)
{
	if (!scene) return;

	for (int y = 0; y < border.Y; ++y)
	{
		for (int x = 0; x < border.X; ++x)
		{
			auto element = GetElement(FCoord2D(x, y));

			if (element == nullptr) return;

			SDL_Rect target = { x * size,y * size,size,size };
			SDL_Rect None_source = { 0,0,16,16 };

			// Source Texture Clip
			SDL_Rect A_source = { 0,0,16,16 };
			SDL_Rect B_source = { 16,0,16,16 };
			SDL_Rect C_source = { 32,0,16,16 };
			SDL_Rect D_source = { 48,0,16,16 };
			SDL_Rect E_source = { 64,0,16,16 };

			SDL_Point center = { size / 2 ,size / 2 };

			if (element->used == false)
			{
				SDL_RenderCopy(renderer, None, &None_source, &target);
			}

			else if (element->used == true)
			{
				switch (element->around)
				{
				case 1: SDL_RenderCopyEx(renderer, Road, &A_source, &target, element->angle, &center, SDL_FLIP_NONE); 
					break;

				case 2: SDL_RenderCopyEx(renderer, Road, element->corner ? &C_source : &B_source, &target, element->angle, &center, SDL_FLIP_NONE);
					break;

				case 3: SDL_RenderCopyEx(renderer, Road, &D_source, &target, element->angle, &center, SDL_FLIP_NONE);
					break;
				case 4: SDL_RenderCopyEx(renderer, Road, &E_source, &target, element->angle, &center, SDL_FLIP_NONE);
					break;

				default: SDL_RenderCopy(renderer, None, &None_source, &target);
				}
			}
			else
			{
				SDL_RenderCopy(renderer, None, &None_source, &target);
			}
		}
	}
}

int main(int argc, char* argv[])
{
	const int dungeonSizeW = 64;
	const int dungeonSizeH = 32;

	Dungeon dungeon;
	dungeon.Reset(FCoord2D(dungeonSizeW, dungeonSizeH), 0x00);

	if (SDL_Init(SDL_INIT_VIDEO) != 0)
	{
		return -1;
	}

	auto window = SDL_CreateWindow("Generator", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, (dungeonSizeW + 1) * 16, (dungeonSizeH + 1) * 16, SDL_WINDOW_SHOWN);
	if (window == nullptr)
	{
		SDL_Quit();

		return -2;
	}

	auto renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
	if (renderer == nullptr)
	{
		SDL_DestroyWindow(window);
		SDL_Quit();

		return -3;
	}

	auto IMG_None = IMG_Load("../assets/None.png");
	auto IMG_Road = IMG_Load("../assets/Road.png");
	auto IMG_Space = IMG_Load("../assets/Space.png");

	if (!IMG_None || !IMG_Road || !IMG_Space)
	{
		return -4;
	}

	auto TEX_None = SDL_CreateTextureFromSurface(renderer, IMG_None);
	auto TEX_Road = SDL_CreateTextureFromSurface(renderer, IMG_Road);
	auto TEX_Space = SDL_CreateTextureFromSurface(renderer, IMG_Space);

	if (!TEX_None || !TEX_Road || !IMG_Space)
	{
		return -5;
	}

	SDL_Event event;
	std::random_device random;

	bool quit = false;
	while (!quit)
	{
		// Event
		while (SDL_PollEvent(&event))
		{
			// Quit
			if (event.type == SDL_QUIT)
			{
				quit = true;
			}

			// Keyboard
			if (event.type == SDL_KEYDOWN)
			{
				dungeon.Reset(FCoord2D(dungeonSizeW, dungeonSizeH), random());
			}

			// Mouse
			if (event.type == SDL_MOUSEBUTTONDOWN)
			{
				dungeon.Reset(FCoord2D(dungeonSizeW, dungeonSizeH), random());
			}
		}

		SDL_RenderClear(renderer);
		SDL_SetRenderDrawColor(renderer, 80, 80, 80, 0xFF);
		SDL_RenderFillRect(renderer, nullptr);

		dungeon.Draw(renderer, TEX_None, TEX_Road, TEX_Space);

		SDL_RenderPresent(renderer);
		SDL_Delay(10);
	}

	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();

	return 0;
}
