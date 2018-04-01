#include <stdio.h>
#include <string.h>
#include <dirent.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
void list_directoare(char *path, int r, long size, int exec ){
	DIR *d = opendir(path);
	struct dirent *direc;
	struct stat info;
	char d_path[255];

inceput:	while ((direc = readdir(d)) != NULL){

			sprintf(d_path,"%s/%s",path,direc->d_name);
			stat(d_path, &info);
			if(exec == 1){
				if(info.st_mode & S_IXUSR){
					goto continuare;	
				}
				else {
					goto inceput;
				}
			}



continuare:	if(direc->d_type != DT_DIR){

			if(size > 1){
				if( (long)info.st_size < size ){ 
					printf("%s/%s\n",path,direc->d_name);
				}
			}
			else 
				printf("%s/%s\n",path,direc->d_name);

		}
			else{
				if(direc->d_type == DT_DIR && strcmp(direc->d_name,".") != 0 && strcmp(direc->d_name,"..") != 0) 
				{
					if (size <= 1){
						printf("%s/%s\n",path,direc->d_name);
					}
					sprintf(d_path,"%s/%s", path, direc->d_name);
					if(r == 1){
						list_directoare(d_path,r,size,exec);
					}
				}
			}

		}

		closedir(d);
}


void  parsare(char *path){
	int version=0;
	int nsections=0;
	int  hsize=0;
	char magic[3];
	int fd;
	char s_name[10];
	int s_type=0;
	int s_size=0;
	int s_offset=0;
	int i;

	fd = open(path,O_RDONLY);
	lseek(fd,0,SEEK_END);

	lseek(fd,-4,SEEK_CUR);
	read(fd,magic,4);
	if(strcmp(magic,"QzD9") != 0){
		printf("ERROR\nwrong magic\n");
		return ;
	}

	lseek(fd,-6,SEEK_CUR);
	read(fd,&hsize,2);
	lseek(fd,0,SEEK_END);
	lseek(fd,-hsize,SEEK_CUR);
	read(fd,&version,4);
	if((version < 37) || (version > 144)){
		printf("ERROR\nwrong version\n");
		return ;
	}

	read(fd,&nsections,1);
	if((nsections < 4) || (nsections > 18)){
		printf("ERROR\nwrong sect_nr\n");
		return ;
	}

	for(i = 0;i<nsections;i++){
		lseek(fd,10,SEEK_CUR);
		read(fd,&s_type,2);
		if((s_type != 58) && (s_type != 72) && (s_type != 60) && (s_type != 42)){
			printf("ERROR\nwrong sect_types\n");
			return ;
		}
		lseek(fd,8,SEEK_CUR);
	}
	lseek(fd,0,SEEK_END);
	lseek(fd,-hsize+5,SEEK_CUR);
	printf("SUCCESS\n");
	printf("version=%d\n",version);
	printf("nr_sections=%d\n",nsections);
	for(i = 0;i < nsections;i++){
		read(fd,s_name,10);
		read(fd,&s_type,2);
		read(fd,&s_offset,4);
		read(fd,&s_size,4);
		printf("section%d: %s %d %d\n",i+1,s_name,s_type,s_size);
	}


}

void  parsare1(char *path,int sect,int *ret){
	int version=0;
	int nsections=0;
	int  hsize=0;
	char magic[3];
	int fd;
	char s_name[10];
	int s_type=0;
	int s_size=0;
	int s_offset=0;
	int i;

	fd = open(path,O_RDONLY);
	lseek(fd,0,SEEK_END);

	lseek(fd,-4,SEEK_CUR);
	read(fd,magic,4);
	if(strcmp(magic,"QzD9") != 0){
		ret[0] = -1;
		return ;
	}

	lseek(fd,-6,SEEK_CUR);
	read(fd,&hsize,2);
	lseek(fd,0,SEEK_END);
	lseek(fd,-hsize,SEEK_CUR);
	read(fd,&version,4);
	if((version < 37) || (version > 144)){
		ret[0] = -1;
		return ;
	}

	read(fd,&nsections,1);
	if((nsections < 4) || (nsections > 18)){
		ret[0] = -1;
		return ;
	}

	for(i = 0;i<nsections;i++){
		lseek(fd,10,SEEK_CUR);
		read(fd,&s_type,2);
		if((s_type != 58) && (s_type != 72) && (s_type != 60) && (s_type != 42)){

			ret[0] = -1;
			return ;
		}
		lseek(fd,8,SEEK_CUR);
	}
	lseek(fd,0,SEEK_END);
	lseek(fd,-hsize+5,SEEK_CUR);

	for(i = 0;i < nsections;i++){
		read(fd,s_name,10);
		read(fd,&s_type,2);
		read(fd,&s_offset,4);
		read(fd,&s_size,4);
		if (i+1 == sect){
			ret[0] = s_size;
			ret[1] = s_offset;
		}


	}


}






void extract(char *path, int s, int l){

	int fd;
	int inf[4];
	int lcomp = 0;
	char buf[2];
	fd = open(path,O_RDONLY);
	parsare1(path,s,inf);
	if (inf[0] == -1){
		printf("ERROR\ninvalid file\n");
		return;
	}
	if (inf[0] == 0){
		printf("ERROR\ninvalid section\n");
		return ;
	}
	printf("SUCCESS\n");
	lseek(fd,0,SEEK_SET);
	lseek(fd,inf[1]+inf[0],SEEK_CUR);
	int j = 1;
	lseek(fd,-2,SEEK_CUR);
	while  (lcomp < l-1){
		read(fd,&buf[0],1);
		read(fd,&buf[1],1);
		lseek(fd,-4,SEEK_CUR);
		if((buf[1] == '\n') || (buf[0] == '\n'))
			lcomp++;
		j++;
	}
	if(buf[1] == '\n')
		printf("%c",buf[0]);
	do{				
		read(fd,&buf[0],1);
		read(fd,&buf[1],1);
		if( (buf[1] != '\n')){
			printf("%c",buf[1]);
			printf("%c",buf[0]);
		}

		lseek(fd,-4,SEEK_CUR);


	}while((buf[1] != '\n') && (buf[0] != '\n'));
	printf("\n");

}
int main(int argc, char **argv){
	int r = 0;
	int exec = 0;
	char *val ;
	int i;
	char *path;
	char *s;
	int section;
	int line;
	char *l;
	long size = 0;
	if(argc >= 2){
		if(strcmp(argv[1],"variant") == 0){
			printf("50299\n");
			return 0;
		}
		else if(strcmp(argv[1],"list") == 0){
			for (i = 1;i < argc;i++){
				if (strcmp(argv[i], "recursive") == 0){ 
					r++;
				}
				if (strstr(argv[i],"size_smaller") != NULL){
					val = strtok(argv[i], "=");
					val = val + 13;
					sscanf(val,"%ld",&size);
				}
				if (strcmp(argv[i], "has_perm_execute") == 0){
					exec++;
				}
				if (strstr(argv[i],"path") != NULL){
					path = strtok(argv[i], "=");
					path = path + 5;
				}

			}
			DIR *d = opendir(path);
			if(d == NULL){
				printf("ERROR\nCalea nu este buna\n");
			}
			else{
				closedir(d);
				printf("SUCCESS\n");
				list_directoare(path, r, size, exec);
				return 0;

			}
		}
		else if (strcmp(argv[1],"parse") == 0){
			if (strstr(argv[2],"path") != NULL){
				path = strtok(argv[2], "=");
				path = path + 5;
			}
			parsare(path);
			return 0;
		}
		else if (strcmp(argv[1],"extract") == 0){
			for ( i = 1;i < argc; i++){
				if (strstr(argv[i],"path") != NULL){
					path = strtok(argv[i], "=");
					path = path + 5;

				}
				if (strstr(argv[i],"section") != NULL){
					s = strtok(argv[i], "=");
					s = s + 8;
					sscanf(s,"%d",&section);
				}
				if (strstr(argv[i],"line") != NULL){
					l = strtok(argv[i], "=");
					l = l + 5;
					sscanf(l,"%d",&line);
				}
			}
			extract(path,section,line);
			return 0;
		}


	}
	return 0;
}
