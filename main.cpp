#include <stdio.h>
#include <stdlib.h>

#define STACK_MAX_SIZE 256
#define  IGCT 8

typedef enum{  // we need enum to identify the object type and holds it
    INT,
    TWIN
}ObjectType;

typedef struct sObject{  // this struct also has a pole type
        ObjectType type;
        unsigned char marked;

        struct sObject* next; // one node in our linked list

        union{  // we need union to contain data about int or twin and all data share the same memory location
            int value;

            struct{
                struct sObject* head;
                struct sObject* tail;
            };
        };
} Object;
// So now we are creating a  virtual machine.
// First of all, we have to make a mechanism for working with memory
// Something like virtual memory. We will have a stack which contain
// all local variables and all temporary variables
// So start with basic stack

typedef struct{
    Object* stack[STACK_MAX_SIZE];
    int stackSize;

    Object* firstObject; // start point in our linked list

    int numObjects;

    int maxObjects;
}VM;


void push(VM* vm, Object* value) // for pushing frames from stack
{
    vm->stack[vm->stackSize++] = value;
}


Object* pop(VM* vm){ // for popping frames from stack
    return vm->stack[--vm->stackSize];
}


VM* newVm() { // func which return initialised stack( kernel ours vm)
    VM* vm = (VM*)malloc(sizeof(VM)); // allocating memory for vm
    vm->stackSize = 0; // initialising the stackSize for vm
    vm->firstObject = NULL; // creating start point for linked list (first object will be null) and the next object we add to our link list in our vm
    vm->numObjects = 0;
    vm->maxObjects = IGCT;
    return vm;
}




// So now we need to learn how to mark objects and theirs poles
// If we will have to work with TWINs at 100% it will be a overflow stack
// because of tone of links from TWINs


void mark(Object* object){ // This func is a root of our GC
    if (object->marked) return; // this is our exit from recursion
    // we are checking the object and if it marked return it
    object->marked = 1; // this bit is responsible for decision of deleting object or not

    if (object->type == TWIN){
        mark(object->head);
        mark(object->tail);
    }
}



void markAll(VM* vm){ // just a func for marking object by our GC in vm
    for (int i = 0; i < vm->stackSize; i++) {
        mark(vm->stack[i]);

    }
}

//  So and we have a problem that in default situation we can't reach all unmarked object
//  We are contain only pointers to object in variables and TWIN poles
//  And when nothing is point on object we have memory leak.
//  So, we can try to fix it with linked list( we will create pointers iside the vm with fixed access) ( I think, we don't need the double linked list)
//  Smth like reference counting mechanism( JVM also has this mechanism)
//  So, I need to write my own linked list which will contain all objects I've ever created
//  the object in linked list will be a node of linked list
// I create the link on the next object (line 15) and a start point (line 39)

void markspeep(VM* vm) { // search and deleting object in this func, where we are deleting object if we can't reach it with our GC
    Object** object = &vm->firstObject;
    while (*object) {
        if (!(*object)->marked) { // if we can't reach object with our GC
            Object* unreached = *object; // we make it unreached
            *object = unreached->next;
            free(unreached); // and deleting it and deleting the memory which was bringing

            vm->numObjects--;
        }
        else {
            (*object)->marked = 0; // if object marked we go to another object
            object = &(*object)->next;
        }
    }
}
// So, and the last func which create a collector
void gc(VM* vm){
    int numObjects = vm->numObjects;

    markAll(vm);
    markspeep(vm);

    vm->maxObjects = vm->numObjects * 2;

    printf("Collected %d objects, %d left.\n", numObjects - vm->numObjects, vm->numObjects);
}


Object* newObject(VM* vm, ObjectType type) // learning to creating objects
{
    if (vm->numObjects == vm->maxObjects) gc(vm);
    Object* object = (Object*)malloc(sizeof(Object)); // allocating memory for new object in vm
    object->type = type; // giving the struct object - type
    object->next = vm->firstObject;
    vm->firstObject = object;
    object->marked = 0;

    vm->maxObjects++;
    return object;
}

void pushInt(VM* vm, int intValue){ // func to custom push an object of INT type
    Object* object = newObject(vm, INT);
    object->value = intValue;
    push(vm, object);
}


Object* pushPair(VM* vm){ //func to custom push an object of TWIN type
    Object* object = newObject(vm, TWIN);
    object->tail = pop(vm);
    object->head = pop(vm);

    push(vm, object);
    return object;

}

void printObj(Object* object){ // just a func for visualisation of test
    switch (object->type) {
        case INT:
            printf("%d", object->value);
            break;

        case TWIN:
            printf("(");
            printObj(object->head);
            printf(", ");
            printObj(object->tail);
            printf(")");
            break;
    }
}


void freeVM(VM* vm){  // func for freeing memory of vm
    vm->stackSize = 0;
    gc(vm);
    free(vm);
}


void first_test(){
    printf("1: Objects in stack are saved.\n");
    VM* vm = newVm();
    pushInt(vm, 1);
    pushInt(vm, 2);

    gc(vm);
    freeVM(vm);
}


void second_test(){
    printf("2: Not reachable objects deleted.\n");
    VM* vm = newVm();
    pushInt(vm, 1);
    pushInt(vm, 2);
    pop(vm);
    pop(vm);

    gc(vm);
    freeVM(vm);
}



void third_test(){
    printf("3: Reach to linked objects.\n");
    VM* vm = newVm();
    pushInt(vm, 1);
    pushInt(vm, 2);
    pushPair(vm);
    pushInt(vm, 3);
    pushInt(vm, 4);
    pushPair(vm);
    pushPair(vm);

    gc(vm);
    freeVM(vm);
}


void perfomance() {
    printf("Perfomance of GC.\n");
    VM* vm = newVm();

    for (int i = 0; i < 1000; i++) {
        for (int j = 0; j < 20; j++) {
            pushInt(vm, i);
        }

        for (int k = 0; k < 20; k++) {
            pop(vm);
        }
    }
    freeVM(vm);
}


int main(int argc, const char** argv){
    first_test();
    second_test();
    third_test();
    perfomance();

    return 0;
}