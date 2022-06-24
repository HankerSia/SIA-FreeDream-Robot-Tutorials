#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <opencv2/opencv.hpp>

#define img_num 2000
char img_file[img_num][1000];

int list_dir_name(char* dirname, int tabs)
{
    DIR* dp;
    struct dirent* dirp;
    struct stat st;
    char tab[tabs + 1];
		char img_count=0;

    /* open dirent directory */
    if((dp = opendir(dirname)) == NULL)
    {
        perror("opendir");
        return -1;
    }



    /* fill tab array with tabs */
    memset(tab, '\t', tabs);
    tab[tabs] = 0;

    /**
     * read all files in this dir
     **/
    while((dirp = readdir(dp)) != NULL)

    {
        char fullname[255];
        memset(fullname, 0, sizeof(fullname));

        /* ignore hidden files */
        if(dirp->d_name[0] == '.')
            continue;

        /* display file name */
        //printf("img_name:%s\n", dirp->d_name);

        strncpy(fullname, dirname, sizeof(fullname));
        strncat(fullname, dirp->d_name, sizeof(fullname));
				strcat(img_file[img_count++], fullname);				
				printf("Image %3d path:%s\n",img_count-1,img_file[img_count-1]);//fullname=dir+file name,the absolute path of the image file        

				/* get dirent status */
        if(stat(fullname, &st) == -1)
        {
            perror("stat");
            fputs(fullname, stderr);
            return -1;
        }
        /* if dirent is a directory, call itself */
        if(S_ISDIR(st.st_mode) && list_dir_name(fullname, tabs + 1) == -1)
            return -1;
    }
    return img_count;

}

int main(int argc, char* argv[])
{
	char* dir="/home/robot/Downloads/mark_recognition/car_img/simple_3class/";
  printf("%s\n", dir);

  char sum=list_dir_name(dir, 1);
	printf("Img total num:%d\n",sum);

	int i;
	char order[1000];

	char txt_path[1000];
	char* txt_name="train.txt";
	memset(txt_path, 0, sizeof(txt_path));
	strcat(txt_path,dir);
	strcat(txt_path,txt_name);
	FILE *fp = fopen(txt_path, "w");

	for (i = 0; i<sum; ++i)
	{
		char img_path[1000];
		memset(img_path, 0, sizeof(img_path));

		printf("Source %s\n", img_file[i]);
		IplImage *pSrc = cvLoadImage(img_file[i]);
		sprintf(order, "%05d.jpg", i);
		strcat(img_path,dir);
		strcat(img_path,order);
		cvSaveImage(img_path, pSrc);
		fprintf(fp, "%05d\n", i);
		
		printf("Save as%s\n", img_path);
		cvReleaseImage(&pSrc);
	}
	fclose(fp);

	

	return 0;
}







