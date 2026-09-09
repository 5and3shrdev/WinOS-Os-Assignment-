#include <stdio.h>
#include <string.h>
#include "filesystem.h"

/*
 * Basic model of an NTFS-style file system: a hierarchy of files and
 * folders is really just a flat table of metadata records, each pointing
 * to its parent -- the same idea as NTFS's Master File Table (MFT), where
 * the folder tree is reconstructed from parent references rather than
 * being stored as an actual tree on disk.
 */

#define MAX_NODES 200
#define NAME_LEN 32

typedef struct {
    char name[NAME_LEN];
    int is_folder;
    int size_kb;
    int parent;   /* -1 for root */
    int used;
} Node;

static Node nodes[MAX_NODES];
static int node_count = 0;
static int current_dir = 0;

static int create_node(const char *name, int is_folder, int size_kb, int parent) {
    if (node_count >= MAX_NODES) return -1;
    int id = node_count++;
    strncpy(nodes[id].name, name, NAME_LEN - 1);
    nodes[id].name[NAME_LEN - 1] = '\0';
    nodes[id].is_folder = is_folder;
    nodes[id].size_kb = size_kb;
    nodes[id].parent = parent;
    nodes[id].used = 1;
    return id;
}

static void reset_fs(void) {
    node_count = 0;
    current_dir = create_node("C:", 1, 0, -1);
}

static void print_path(int id) {
    if (id == -1) return;
    if (nodes[id].parent != -1) { print_path(nodes[id].parent); printf("\\"); }
    printf("%s", nodes[id].name);
}

static void list_dir(int dir) {
    printf("\n  Directory of "); print_path(dir); printf("\n\n");
    int any = 0;
    for (int i = 0; i < node_count; i++) {
        if (nodes[i].used && nodes[i].parent == dir) {
            any = 1;
            if (nodes[i].is_folder) printf("    <DIR>       %s\n", nodes[i].name);
            else printf("    %6d KB  %s\n", nodes[i].size_kb, nodes[i].name);
        }
    }
    if (!any) printf("    (empty)\n");
}

static int find_child(int dir, const char *name) {
    for (int i = 0; i < node_count; i++)
        if (nodes[i].used && nodes[i].parent == dir && strcmp(nodes[i].name, name) == 0) return i;
    return -1;
}

static void delete_recursive(int id) {
    for (int i = 0; i < node_count; i++)
        if (nodes[i].used && nodes[i].parent == id) delete_recursive(i);
    nodes[id].used = 0;
}

static void print_mft(void) {
    printf("\n  MFT-style record table (one metadata record per file/folder):\n");
    printf("  %-4s %-16s %-8s %-8s %-8s\n", "ID", "Name", "Type", "SizeKB", "ParentID");
    for (int i = 0; i < node_count; i++) {
        if (!nodes[i].used) continue;
        printf("  %-4d %-16s %-8s %-8d %-8d\n", i, nodes[i].name,
               nodes[i].is_folder ? "Folder" : "File", nodes[i].is_folder ? 0 : nodes[i].size_kb, nodes[i].parent);
    }
}

static void interactive_menu(void) {
    int choice;
    do {
        printf("\n  Current directory: "); print_path(current_dir); printf("\n");
        printf("  1.Create folder  2.Create file  3.Delete  4.List dir  5.Enter folder\n");
        printf("  6.Go up (..)     7.Show MFT table  0.Back\n  Choice: ");
        if (scanf("%d", &choice) != 1) break;

        char name[NAME_LEN];
        int id, size;
        switch (choice) {
            case 1:
                printf("  Folder name: "); scanf("%31s", name);
                if (find_child(current_dir, name) != -1) printf("  Already exists.\n");
                else create_node(name, 1, 0, current_dir);
                break;
            case 2:
                printf("  File name: "); scanf("%31s", name);
                printf("  Size (KB): "); scanf("%d", &size);
                if (find_child(current_dir, name) != -1) printf("  Already exists.\n");
                else create_node(name, 0, size, current_dir);
                break;
            case 3:
                printf("  Name to delete: "); scanf("%31s", name);
                id = find_child(current_dir, name);
                if (id == -1) printf("  Not found.\n");
                else { delete_recursive(id); printf("  Deleted.\n"); }
                break;
            case 4:
                list_dir(current_dir);
                break;
            case 5:
                printf("  Folder to enter: "); scanf("%31s", name);
                id = find_child(current_dir, name);
                if (id == -1 || !nodes[id].is_folder) printf("  No such folder.\n");
                else current_dir = id;
                break;
            case 6:
                if (nodes[current_dir].parent != -1) current_dir = nodes[current_dir].parent;
                else printf("  Already at root.\n");
                break;
            case 7:
                print_mft();
                break;
        }
    } while (choice != 0);
}

void filesystem_run_demo(void) {
    reset_fs();
    printf("\n  Demo: building a small NTFS-style folder tree...\n");

    int docs = create_node("Documents", 1, 0, current_dir);
    int pics = create_node("Pictures", 1, 0, current_dir);
    create_node("resume.docx", 0, 45, docs);
    create_node("notes.txt", 0, 3, docs);
    create_node("vacation.jpg", 0, 2200, pics);

    list_dir(current_dir);
    printf("\n  Entering Documents...\n");
    list_dir(docs);
    print_mft();

    printf("\n  Notice every file/folder is one flat record with a parent pointer --\n");
    printf("  that's the same idea NTFS uses in its Master File Table.\n");
}

void filesystem_run_interactive(void) {
    reset_fs();
    printf("\n  Build your own folder tree using the menu below.\n");
    interactive_menu();
}
