#include <stdio.h>
#include <stdlib.h>

int main()
{
	//load "new1.ppm"
    FILE* f;
    f = fopen("new1.ppm", "rb");
    if(!f)
        return 1;
    int x, y, max;
    fscanf(f, "%*s %d %d %d", &x, &y, &max);
    fseek(f, 1, SEEK_CUR);
    printf("%d %d %d", x, y, max);
    unsigned char * data = (unsigned char *)malloc(x*y*3*sizeof(unsigned char));
    fread(data, x*y*3, sizeof(char), f);
    fclose(f);
	//allocate memory for resulting image
    unsigned char * (unsigned char *)newdata = calloc(x*y*3, sizeof(unsigned char));
	//create filter mask
    int mask[3][3] =
    {
        {1, 1, 1},
        {1, 0, 1},
        {1, 1, 1}
    };
	//calculate weight of the mask
	int weight = 0;
	for(int mi = 0; mi < 3; mi++)
        for(int mj = 0; mj < 3; mj++)
            weight += mask[mi][mj];
	//apply filter for whole image
	#pragma omp parallel for 
    for(int i = 1; i < y - 1; i++)
        for(int j = 3; j < x*3 - 3; j+=3)
            for(int shift = 0; shift < 3; shift++)//calculate pixel colors
			{
				//calculate each value of pixel components (R, G, B)
				unsigned int tmp = 0;
				for(int mi = 0; mi < 3; mi++)
					for(int mj = 0; mj < 3; mj++)
						tmp += mask[mi][mj]*data[(i+(mi-1))*y*3+(j+(mj-1)*3)+shift];
				newdata[i*y*3+j+shift] = (char)(((float)tmp)/weight + 0.5);//save color value
			}
	//save the result as "new2.ppm"
    f = fopen("new2.ppm", "wb");
    if(!f)
        return 1;
    fprintf(f, "P6 %d %d %d ", x, y, max);
    fwrite(newdata, x*y*3, sizeof(unsigned char), f);
    free(data);
    fclose(f);
    return 0;
}
