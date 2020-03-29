/*************************************************************
** class name:  OperationList
**
** description: Record operations(Move, Delete, rotate features)
**              Support undo and redo.
**
** last change: 2020-03-26
**************************************************************/
#ifndef OPERATIONLIST_H
#define OPERATIONLIST_H

#include "operation/operation.h"

class OperationList
{
public:
    OperationList();
    ~OperationList();

public:
    void setOpenglWidget() {}

    void setMaxOperations(int maxSize);
    void clear();

    bool isAtBeginning() const { return (!isOverrided) && (curr - 1 == first); }

    void addOperation(Operation* operation);
    Operation* undo();
    Operation* redo();

    Operation* addDeleteOperation(const FeaturesList& features);
    Operation* addMoveOperation(const FeaturesList& features, double xOffset, double yOffset);
    Operation* addRotateOperation(const FeaturesList& features, double angle);

private:
    /* Record 20 steps at most, change by call setMaxOperations(int) */
    Operation** list = nullptr;
    int MAX_SIZE = 20;

    int curr = 1;
    int first = 0;

    bool isOverrided = false;
};

#endif // OPERATIONLIST_H
