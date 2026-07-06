#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>

#include "btree.h"
#include "page.h"

static void print_usage(const char* prog){
    fprintf(stderr,
        "Usage:\n"
        "  %s <dbfile> init\n"
        "  %s <dbfile> insert <value> [<value> ...]\n"
        "  %s <dbfile> get <key> [<key> ...]\n"
        "  %s <dbfile> update <key> <new_value>\n"
        "  %s <dbfile> delete <key> [<key> ...]\n",
        prog, prog, prog, prog, prog);
}

int main(int argc, char** argv){
    if(argc < 3){
        print_usage(argv[0]);
        return 1;
    }

    const char* dbfile = argv[1];
    const char* cmd = argv[2];

    if(strcmp(cmd, "init") == 0){
        PageManager* pm = pagemanager_create(dbfile, true);
        if(pm == NULL){
            fprintf(stderr, "Failed to create DB file: %s\n", dbfile);
            return 1;
        }
        // Create a B-tree to ensure the root page is initialized (already done by PageManager)
        Btree* t = btree_create(pm);
        (void)t;
        printf("Initialized new DB: %s\n", dbfile);
        close(pm->fd);
        return 0;
    }

    // All other commands require opening an existing DB
    PageManager* pm = pagemanager_open(dbfile);
    if(pm == NULL){
        fprintf(stderr, "Failed to open DB file: %s (did you run 'init'?)\n", dbfile);
        return 1;
    }
    Btree* t = btree_create(pm);

    if(strcmp(cmd, "insert") == 0){
        if(argc < 4){
            fprintf(stderr, "insert requires at least one <value>\n");
            print_usage(argv[0]);
            close(pm->fd);
            return 1;
        }
        int inserted = 0;
        for(int i = 3; i < argc; i++){
            int value = atoi(argv[i]);
            btree_insert(t, value, NULL, NULL);
            inserted++;
        }
        printf("Inserted %d value(s). Keys are auto-assigned.\n", inserted);
        close(pm->fd);
        return 0;
    }

    if(strcmp(cmd, "get") == 0){
        if(argc < 4){
            fprintf(stderr, "get requires at least one <key>\n");
            print_usage(argv[0]);
            close(pm->fd);
            return 1;
        }
        for(int i = 3; i < argc; i++){
            int key = atoi(argv[i]);
            int value = btree_find(t, key, NULL);
            if(value == 0){
                printf("key %d -> <not found>\n", key);
            }else{
                printf("key %d -> %d\n", key, value);
            }
        }
        close(pm->fd);
        return 0;
    }

    if(strcmp(cmd, "update") == 0){
        if(argc != 5){
            fprintf(stderr, "update requires <key> <new_value>\n");
            print_usage(argv[0]);
            close(pm->fd);
            return 1;
        }
        int key = atoi(argv[3]);
        int new_value = atoi(argv[4]);
        bool ok = btree_update(t, key, new_value, NULL);
        if(ok){
            printf("Updated key %d -> %d\n", key, new_value);
        }else{
            printf("Key %d not found.\n", key);
        }
        close(pm->fd);
        return ok ? 0 : 2;
    }

    if(strcmp(cmd, "delete") == 0){
        if(argc < 4){
            fprintf(stderr, "delete requires at least one <key>\n");
            print_usage(argv[0]);
            close(pm->fd);
            return 1;
        }
        int deleted = 0;
        for(int i = 3; i < argc; i++){
            int key = atoi(argv[i]);
            bool ok = btree_delete(t, key, NULL, NULL);
            if(ok){
                deleted++;
                printf("Deleted key %d\n", key);
            }else{
                printf("Key %d not found or delete failed\n", key);
            }
        }
        printf("Deleted %d key(s).\n", deleted);
        close(pm->fd);
        return 0;
    }

    fprintf(stderr, "Unknown command: %s\n", cmd);
    print_usage(argv[0]);
    close(pm->fd);
    return 1;
}
