#include <stdio.h>

#include "file_node.h"

#include "personality.h"

#include "report.h"

#include "dialogue_view.h"
 
int main(void) {

    FileNode* head = NULL;
 
    appendFileNode(&head, createFileNode("homework.c", "c", 3200));

    appendFileNode(&head, createFileNode("old_photo.png", "png", 2480000));

    appendFileNode(&head, createFileNode("mysterious.tmp", "tmp", 800));

    appendFileNode(&head, createFileNode("final_report.docx", "docx", 145000));

    appendFileNode(&head, createFileNode("game.exe", "exe", 7300000));
 
    assignPersonalities(head);
 
    printFileList(head);
 
    showPopupDialogues(head);
 
    freeFileList(head);
 
    return 0;

}
 