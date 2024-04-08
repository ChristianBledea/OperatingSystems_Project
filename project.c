#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <stdlib.h>

#define MAX_FILES 100

struct file_info{
    char* name;
    long id;
    long size;

}

void parse_folder(char* name, int indent){
    DIR* parent=opendir(name);
    struct dirent* file=readdir(parent);
    char[1024] path;
    while(file!=NULL){
        if(strcmp(file->d_name, ".")||strcmp(file->d_name, "..")){
            continue;
        }
        snprintf(path, sizeof(path), "%s/%s", name, file->d_name);
        printf("%*s[%s]\n", indent, "", file->d_name);
        parse_folder(path, indent+2);
        struct stat* st;
        if (stat(path, &st) == -1) {
            perror("Error getting file information");
            return 1;
        }
        save_lstat_info("output.txt",file->d_name ,st);

    }
   
    
    closedir(parent);
}

struct file_info* create_data(char* file){
    int fd = open(file, O_RDONLY);
    if (fd == -1) {
        perror("Error opening file");
        return;
    }

    char buffer[1024];
    struct file_info* files=malloc(MAX_FILES*sizeof(struct file_info));
    
    while (1) {
        ssize_t bytes_read = read(fd, buffer, sizeof(buffer));
        if (bytes_read <= 0)
            break;

        char *start = buffer;
        char *end = buffer;

        while (end - buffer < bytes_read) {
            if (*end == ',' || *end == '\n') {
                *end = '\0'; 
                if (*start != '\0') {
                    
                    char *name = strdup(start);
                    char *id_str = strtok(NULL, ",");
                    char *size_str = strtok(NULL, ",");
                    int i=0;
                    if (name != NULL && id_str != NULL && size_str != NULL) {
                        
                        files[i].name = name;
                        files[i].id = atoll(id_str);
                        files[i].size = atoll(size_str);
                        i++;
                    }
                }
                start = end + 1;
            }
            end++;
        }

    }
    return files;
    close(fd);
}


void save_lstat_info(const char *filename,const char* name, const struct stat *st) {
    int fd = open(filename, O_WRONLY | O_CREAT | O_APPEND, 0644);
    if (fd == -1) {
        perror("Error opening file");
        return;
    }

    char buffer[256];
    int n;

    n = snprintf(buffer, sizeof(buffer), "%s,", name);
    n = snprintf(buffer, sizeof(buffer), "%ld,", st->st_size);
    n = snprintf(buffer, sizeof(buffer), "%ld,", st->st_ino);
    write(fd, buffer, n);


}

int count_entries(struct file_info* files){
    int i=0;
    while(files[i]!=NULL){
        i++;
    }
    return i;
}

void snapshot_compare(struct file_info * s1, struct file_info * s2){
    int n=count_entries(s1);
    int m=count_entries(s2);
    int flag=0;
    for(int i=0; i<n; i++){
        for(int j=0; j<m && !flag; j++){
            if(s1[i].id == s2[j].id){
                if(s1[i].size != s2[j].size || strcomp(s1[i].name, s2[j].name))printf("a modification was made for file with id %d", s1[i].id);
                flag=1;
            }
        }
    }
}

int main(int argc, char** argv){

    if(int fd = open("output.txt", O_WRONLY | O_APPEND, 0644)){
        struct file_info* snapshot1=create_data("outpur.txt");
        fd = open("output.txt", O_TRUNC, 0644)
        parse_folder(argv[1]);
        struct file_info* snapshot2=create_data("output.txt");
        snapshot_compare(snapshot1, snapshot2);
        free(snapshot1);
        free(snapshot2);
    }
    else{
        parse_folder(argv[1]);
    }
    
    
    return 0;
}
