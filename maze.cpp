//This program uses OMP tasks for parallel maze solving and visualization of this process.
//compile: g++ maze.cpp -std=c++11 -fopenmp

#include <iostream>
#include <omp.h>
#include <iomanip>
#include <chrono>
#include <thread>

class Maze
{
private:
	int id = 1;//counter for threads IDs
	omp_lock_t id_mutex;//mutex for "id" counter modifications
	int start_x = 2;//entrance position
	int start_y = 0;//entrance position
	static const int h = 20;//maze dimensions
	static const int w = 20;//maze dimensions
	omp_lock_t **maze_mutex;//mutex array for each cell
	std::string solution;//string containing solution path
	
	enum
	{
		X = 0,//wall
		O = -1//path,
	};
	
	int maze[h][w] =
		{        //entrance
			{X,X,O,X,X,X,X,X,X,X,X,X,X,X,X,X,X,X,X,X},
			{X,X,O,X,X,X,X,X,X,X,X,X,X,X,X,X,X,X,X,X},
			{X,X,O,O,O,O,O,O,O,O,O,O,O,O,O,O,X,X,X,X},
			{X,X,X,X,X,X,X,X,X,X,X,X,X,X,X,O,X,X,X,X},
			{X,X,X,X,X,X,X,X,X,X,X,X,X,X,X,O,O,O,O,X},
			{X,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,X},
			{X,X,X,X,O,X,X,X,X,X,X,X,X,X,X,O,O,O,O,X},
			{X,X,X,X,O,X,X,X,X,X,X,X,X,X,X,X,X,X,X,X},
			{X,O,X,X,O,X,X,X,X,X,X,O,O,O,O,O,O,O,O,O},//exit1 (closest to the entrance)
			{X,O,O,O,O,X,O,X,O,O,O,O,X,X,X,X,O,X,X,X},
			{X,X,X,X,O,O,O,X,X,X,X,O,X,O,O,X,O,X,X,X},
			{X,X,X,X,X,O,X,X,X,X,X,O,X,X,O,X,O,X,X,X},
			{X,X,X,X,X,O,X,X,X,X,X,O,O,O,O,X,O,X,X,X},
			{X,X,O,O,O,O,O,O,O,X,X,X,X,X,O,X,O,X,X,X},
			{X,X,O,X,X,X,X,X,O,X,X,X,X,X,O,X,O,X,X,X},
			{X,X,O,X,X,X,X,X,O,X,X,X,X,X,O,X,O,X,X,X},
			{X,X,O,O,O,O,O,O,O,X,X,X,X,X,O,X,O,X,X,X},
			{X,X,X,X,X,O,X,X,X,X,X,X,X,X,O,X,O,O,O,O},//exit2
			{X,X,X,X,X,O,O,O,O,O,O,O,O,O,O,X,X,X,X,X},
			{X,X,X,X,X,X,X,X,X,X,X,X,X,X,X,X,X,X,X,X}
		};
public:
	Maze()
	{
		omp_init_lock(&id_mutex);
		
		maze_mutex = new omp_lock_t*[w];
		for (int i = 0; i < w; i++)
			maze_mutex[i] = new omp_lock_t[h];
		
		for (int i = 0; i < w; i++)
			for (int j = 0; j < h; j++)
				omp_init_lock(&maze_mutex[i][j]);
	}
	
	~Maze()
	{
		for (int i = 0; i < w; i++)
			delete[] maze_mutex[i];
		delete[] maze_mutex;
	}
	
	void solve()
	{
		bool keep_drawing = true, keep_searching = true;
		std::string path;
		#pragma omp parallel sections
		{
			#pragma omp section
			{
				move(start_y, start_x, 1, keep_searching, path);
				keep_drawing = false;
			}
			
			#pragma omp section
			{
				draw(keep_drawing);
			}
		}
	}
	
	std::string get_solution()
	{
		return solution;
	}
private:
	void draw(bool &keep_drawing)
	{
		while (keep_drawing)
		{
			system("cls");
			for (int i = 0; i < h; i++)
			{
				for (int j = 0; j < w; j++)
					if(maze[i][j] == O)
						std::cout << "  ";
					else if(maze[i][j] == X)
						std::cout << static_cast<char>(219) << static_cast<char>(219);
					else
						std::cout << std::setw(2) << std::setfill(' ') << maze[i][j];
				std::cout << std::endl;
			}
			std::this_thread::sleep_for(std::chrono::milliseconds(70));
		}
	}

	void move(int pos_y, int pos_x, int dir, bool &keep_searching, std::string path)//dir: up = 1, down = 2, left = 3, right = 4
	{
		bool up, down, left, right;
		int branches = 1;
		int tmpx, tmpy, tmpdir, local_id;
		omp_set_lock(&id_mutex);
		local_id = id;
		id++;
		omp_unset_lock(&id_mutex);
		while(branches)
		{
			omp_set_lock(&maze_mutex[pos_y][pos_x]);
			if(maze[pos_y][pos_x] == O)
			{
				maze[pos_y][pos_x] = local_id;
				omp_unset_lock(&maze_mutex[pos_y][pos_x]);
			}
			else
			{
				omp_unset_lock(&maze_mutex[pos_y][pos_x]);
				break;
			}
			if(pos_x != start_x && pos_y != start_y && (pos_x + 1 >= w || pos_x - 1 < 0 || pos_y + 1 >= h || pos_y - 1 < 0))
			{
				keep_searching = false;
				solution = path;
			}
			if(!keep_searching)
				break;
			std::this_thread::sleep_for(std::chrono::milliseconds(100));
			branches = 0;
			tmpx = 0;
			tmpy = 0;
			tmpdir = 0;
			up = false;
			down = false;
			left = false;
			right = false;
			
			if(pos_y + 1 < h)//borders check
			{
				omp_set_lock(&maze_mutex[pos_y + 1][pos_x]);//avoid threads collisions
				if (maze[pos_y + 1][pos_x] == O)//if there is a path
				{
					up = true;
					branches++;
					tmpdir = 1;
				}
				omp_unset_lock(&maze_mutex[pos_y + 1][pos_x]);
			}
			
			if(pos_x + 1 < w)//borders check
			{
				omp_set_lock(&maze_mutex[pos_y][pos_x + 1]);//avoid threads collisions
				if (maze[pos_y][pos_x + 1] == O)//if there is a path
				{
					right = true;
					branches++;
					tmpdir = 4;
				}
				omp_unset_lock(&maze_mutex[pos_y][pos_x + 1]);
			}
			
			if(pos_y - 1 >= 0)//borders check
			{
				omp_set_lock(&maze_mutex[pos_y - 1][pos_x]);//avoid threads collisions
				if (maze[pos_y - 1][pos_x] == O)//if there is a path
				{
					down = true;
					branches++;
					tmpdir = 2;
				}
				omp_unset_lock(&maze_mutex[pos_y - 1][pos_x]);
			}
			
			if(pos_x - 1 >= 0)//borders check
			{
				omp_set_lock(&maze_mutex[pos_y][pos_x - 1]);//avoid threads collisions
				if (maze[pos_y][pos_x - 1] == O)//if there is a path
				{
					left = true;
					branches++;
					tmpdir = 3;
				}
				omp_unset_lock(&maze_mutex[pos_y][pos_x - 1]);
			}
			
			if(branches == 0)
				break;
			
			if(branches == 1)//if there is no fork, we will not create a new thread
			{
				dir = tmpdir;
				if(up)
					pos_y++;
				else if(down)
					pos_y--;
				else if(left)
					pos_x--;
				else
					pos_x++;
				continue;
			}
			#pragma omp task shared(keep_searching)
				if(up)
					move(pos_y + 1, pos_x, 1, keep_searching, path + "UP, ");
			
			#pragma omp task shared(keep_searching)
				if(down)
					move(pos_y - 1, pos_x, 2, keep_searching, path + "DOWN, ");
			
			#pragma omp task shared(keep_searching)
				if(left)
					move(pos_y, pos_x - 1, 3, keep_searching, path + "LEFT, ");
			
			#pragma omp task shared(keep_searching)
				if(right)
					move(pos_y, pos_x + 1, 4, keep_searching, path + "RIGHT, ");
			#pragma omp taskwait
		}
	}
};

int main()
{
	omp_set_num_threads(50);
	Maze maze;
	maze.solve();
	std::cout << "Solution: " << maze.get_solution() << std::endl;
    return 0;
}