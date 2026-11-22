#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/stat.h>
#define GRN   "\x1B[32;1m"      //light green
#define RESET "\x1B[0m"         //back to the default color

int my_strcmp(const char *s1, const char *s2) {
    const unsigned char *p1 = (const unsigned char *)s1;
    const unsigned char *p2 = (const unsigned char *)s2;

    while (*p1 == *p2 && *p1 != '\0') {
        p1++;
        p2++;
    }
    return (*p1 > *p2) - (*p1 < *p2);
}

int my_strlen(char *str){
  int i = 0;
  for (i = 0; str[i] != '\0'; i++){}
  return i; 
}

char *my_strcpy(char *dest, const char *src) {
    int i = 0;
    while (src[i] != '\0') {
        dest[i] = src[i];
        i++;
    }
    dest[i] = '\0';
    return dest;
} 

// One directory entry: its name and its last modification time
typedef struct s_file {
    char *name;
    struct timespec mtime;
} t_file;

// Returns < 0 if a must be printed before b, > 0 if after.
// by_time == 1 : newest first (like ls -t), ties broken by name
// by_time == 0 : alphabetical order (like plain ls)
int compare(const t_file *a, const t_file *b, int by_time){ //compares first file with next file
    if (by_time) {
        if (a->mtime.tv_sec != b->mtime.tv_sec)             // compare seconds first
            return (a->mtime.tv_sec > b->mtime.tv_sec) ? -1 : 1;    //if time of file a is > than time of file b then return -1 else return 1
        if (a->mtime.tv_nsec != b->mtime.tv_nsec)           // then nanoseconds
            return (a->mtime.tv_nsec > b->mtime.tv_nsec) ? -1 : 1; //same but with nanoseconds
    }
    return my_strcmp(a->name, b->name);                        // same time (or no -t): sort by name
}

// Simple insertion sort on the array of entries
void sort_files(t_file *files, int count, int by_time){
    for (int i = 1; i < count; i++) {
        t_file key = files[i];                              // element to insert
        int j = i - 1;
        while (j >= 0 && compare(&files[j], &key, by_time) > 0) {
            files[j + 1] = files[j];                        // shift bigger elements to the right
            j--;
        }
        files[j + 1] = key;                                 // put the element at its place
    }
}

int main(int argc, char **argv){
    int show_all = 0;                                       // 1 if -a was given
    int by_time = 0;                                        // 1 if -t was given
    int status = 0;                                         // 0 = ok, 1 = an error happened

    // ---- 1. Parse the options once ----
    for (int i = 1; i < argc; i++) {
        if (argv[i][0] != '-' || argv[i][1] == '\0')        // not an option, ignore it
            continue;  
        for (int j = 1; argv[i][j] != '\0'; j++) {          // walk through grouped flags: -at = -a + -t
            if (argv[i][j] == 'a')
                show_all = 1;
            else if (argv[i][j] == 't')
                by_time = 1;
            else {
                fprintf(stderr, "Unknown option: -%c\n", argv[i][j]); //fprint is same as printf but for files
                return 1;                                   // nothing allocated yet, safe to return
            }
        }
    }

    // ---- 2. Open the current directory ----
    DIR *dp = opendir(".");
    if (!dp) {
        perror("Error opening directory");                  // perror prints our message + the system error
        return 1;
    }

    t_file *files = NULL;                              // dynamic array of entries
    int count = 0;
    int capacity = 0;
    struct dirent *entry;

    // ---- 3. Read every entry and store it ----
    while ((entry = readdir(dp)) != NULL) {
        if (!show_all && entry->d_name[0] == '.'){           // hidden files (and . ..) only with -a
            continue;
        }

        if (count == capacity) {                            // array full: grow it
            capacity = (capacity == 0) ? 16 : capacity * 2;
            t_file *tmp = realloc(files, capacity * sizeof(t_file));
            if (!tmp) {
                perror("realloc failed");
                status = 1;                                 // stop reading, cleanup happens below
                break;
            }
            files = tmp;
        }

        struct stat sb;
        if (lstat(entry->d_name, &sb) == -1) {              // get the file's stats (does not follow symlinks)
            perror("lstat failed");
            status = 1;
            break;
        }

        files[count].name = malloc(my_strlen(entry->d_name) + 1);
        if (!files[count].name) {
            perror("malloc failed");
            status = 1;
            break;
        }
        my_strcpy(files[count].name, entry->d_name);           // copy: entry is overwritten by the next readdir
        files[count].mtime = sb.st_mtim;                    // last modification time (sec + nsec)
        count++;
    }
    closedir(dp);

    // ---- 4. Sort and print (only if everything went fine) ----
    if (status == 0) {
        sort_files(files, count, by_time);                  // alphabetical, or by mtime with -t
        for (int i = 0; i < count; i++)
            printf(GRN "%s" RESET "\n", files[i].name);                  // one name per line in green
    }

    // ---- 5. Single cleanup, used by both the success and error paths ----
    for (int i = 0; i < count; i++)
        free(files[i].name);
    free(files);                                            // free(NULL) is safe if nothing was stored

    return status;
}
