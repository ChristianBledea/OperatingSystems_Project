#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/wait.h>

#define MAX_FILES 100

struct file_info{
    char name[100];
    long long id;
    int size;

};

void listdir(const char *name, struct file_info* files, int *count)
{
    DIR *dir;
    struct dirent *entry;

    if (!(dir = opendir(name)))
        return;

    while ((entry = readdir(dir)) != NULL) {
        
            char path[1024];
            if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
                continue;
                   
            

            
            snprintf(path, sizeof(path), "%s/%s", name, entry->d_name);
            //printf("%s\n", path);
            printf("%s\n", entry->d_name);

            
            
            struct stat st;
            
            int s=stat(path, &st);
            if (s == -1) {
                perror("Error getting file information");
                
                return ;
            }
            
        
            strcpy(files[*count].name, entry->d_name);
            files[*count].size=st.st_size;
            files[*count].id=st.st_ino;


            
            (*count)++;
       
    }
    closedir(dir);
}

void save(struct file_info* files, int count, const char* filename){
    FILE* f=fopen(filename, "w");
    if(f==NULL){
        printf("error when opening file\n");
        exit(EXIT_FAILURE);
    }
    for(int i=0; i<count; i++){
        fprintf(f, "%s,%ld,%lld,", files[i].name, (long)files[i].size, (long long)files[i].id);
    }
    fclose(f);
}


int count_entries(struct file_info* files){
    int i=0;
    while(files[i].name != NULL || files[i].id != 0){
        i++;
    }
    return i;
}

void snapshot_compare(struct file_info * s1, struct file_info * s2, int c1, int c2){
    int flag=0;
    for(int i=0; i<c1; i++){
        for(int j=0; j<c2 && !flag; j++){
            if(s1[i].id == s2[j].id){
                if(s1[i].size != s2[j].size)printf("a modification was made for file with id %lld\n", s1[i].id);
                if(!strcmp(s1[i].name, s2[i].name))printf("file with inode id %lld was renamed\n", s1[i].id);
                flag=1;
            }
        }
        if(!flag)printf("file with inode id %lld was deleted\n", s1[i].id);
    }
}

int permissions(const char *filename) {
    struct stat st;
    
    if (stat(filename, &st) == -1) {
        perror("Error getting file status");
        return -1; // Return -1 to indicate an error
    }
    
    // Check if the file has no permissions
    if ((st.st_mode & S_IRWXU) == 0 && 
        (st.st_mode & S_IRWXG) == 0 && 
        (st.st_mode & S_IRWXO) == 0) {
        return 1; // File has no permissions
    }
    
    return 0; // File has permissions
}

int main(int argc, char** argv){
    
    if(strcmp(argv[1], "-o")!=0){
        printf("please provide output file");
        exit(EXIT_FAILURE);
        
    }
    // if(strcmp(argv[3], "-s")!=0){
    //     printf("please provide isolated_space diectory");
    //     exit(EXIT_FAILURE);
        
    // }
    char output_dir[1024];
    char isolated_sapce_dir[1024];
    pid_t children[10];
    struct file_info* files=malloc(MAX_FILES*sizeof(struct file_info));
    int count=0;
    snprintf(output_dir, sizeof(output_dir), "%s", argv[2]);
    int n=argc-3;
    
    for(int i=0; i<n; i++){
        children[i]=fork();
        if(children[i]<0){
            printf("failed to create another process");
            exit(EXIT_FAILURE);
        }
        else if(children[i]==0){
            printf("child process for directory %s, with pid %d\n", argv[3+i], getpid());
        
            struct file_info* files=malloc(MAX_FILES*sizeof(struct file_info));
            int count=0;
            char path[1024];
            snprintf(path, sizeof(path), "snapshot%d.txt", i);
            printf("%s\n", path);
            listdir(argv[i+3], files, &count);
            // printf("%d\n", count);
            // for(int i=0; i<count; i++){
            //     printf("a");
            //     printf("%s,%ld,%lld,", files[i].name, (long)files[i].size, (long long)files[i].id);
            // }
            save(files, count, path);
            
            printf("Snapshot created for %s\n", argv[i+3]);
            exit(EXIT_SUCCESS);
        }

    }
    int status;
    for(int i=0; i<n; i++){
        waitpid(children[i], &status, 0);
        if(WIFEXITED(status)){
            printf("Child process terminated with PID %d and exit code %d. \n", children[i], WEXITSTATUS(status));
        }
    }

    // listdir(".", 0, files, &count);
    // save(files, count, "output.txt");

    
    return 0;
}