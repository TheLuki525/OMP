#include <iostream>
#include <fstream>

int main()
{
    std::fstream file;
    file.open("new1.ppm", std::ios::in | std::ios::binary);
    if(!file.good())
        return 1;
    int x, y, max;
    std::string P6;
    file >> P6 >> x >> y >> max;
    file.seekg(1, std::ios::cur);
    std::cout << x << " " << y << " " << max << "\n";
    unsigned char *data = new unsigned char[3*x*y]{0};
    file.read((char*)data, 3*x*y);
    file.close();
    unsigned char *newdata = new unsigned char[3*x*y]{0};
    int mask[3][3] =
    {
        {1, 1, 1},
        {1, 0, 1},
        {1, 1, 1}
    };
	int weight = 0;
	for(int mi = 0; mi < 3; mi++)
        for(int mj = 0; mj < 3; mj++)
            weight += mask[mi][mj];
    for(int i = 1; i < y - 1; i++)
        for(int j = 3; j < x*3 - 3; j+=3)
            for(int shift = 0; shift < 3; shift++)
			{
				unsigned int tmp = 0;
				for(int mi = 0; mi < 3; mi++)
					for(int mj = 0; mj < 3; mj++)
						tmp += mask[mi][mj]*data[(i+(mi-1))*y*3+(j+(mj-1)*3)+shift];
				newdata[i*y*3+j+shift] = (char)(((float)tmp)/weight + 0.5);
			}
    file.open("new2.ppm", std::ios::out | std::ios::binary);
    if(!file.good())
        return 1;
    file << "P6 " << x << " " << y << " " << max << " ";
    file.write((char*)newdata, 3*x*y);
    delete[] newdata;
    file.close();
    delete[] data;
    return 0;
}
