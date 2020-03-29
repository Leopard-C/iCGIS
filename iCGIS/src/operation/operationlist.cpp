#include "operation/operationlist.h"
#include "geo/map/geomap.h"
#include "util/env.h"


OperationList::OperationList()
{
    list = new Operation*[MAX_SIZE];
    for (int i = 0; i < MAX_SIZE; ++i) {
        list[i] = nullptr;
    }
}

OperationList::~OperationList() {
    clear();
    delete[] list;
}

void OperationList::setMaxOperations(int maxSize) {
    clear();
    delete[] list;
    MAX_SIZE = maxSize;
    list = new Operation*[MAX_SIZE];
    for (int i = 0; i < MAX_SIZE; ++i) {
        list[i] = nullptr;
    }
}

void OperationList::addOperation(Operation* operation) {
    if (curr == first) {
        first = (first+1) % MAX_SIZE;
        isOverrided = true;
        if (list[curr]) {
            delete list[curr];
            list[curr] = nullptr;
        }
    }
    else if (curr > first) {
        for (int i = 0; i < first; ++i) {
            if (list[i]) {
                delete list[i];
                list[i] = nullptr;
            }
            else {
                break;
            }
        }
        for (int i = curr; i < MAX_SIZE; ++i) {
            if (list[i]) {
                delete list[i];
                list[i] = nullptr;
            }
            else {
                break;
            }
        }
    }
    else {
        for (int i = curr; i < first; ++i) {
            if (list[i]) {
                delete list[i];
                list[i] = nullptr;
            }
            else {
                break;
            }
        }
    }

    list[curr] = operation;
    curr = (curr+1) % MAX_SIZE;

    // Operate!
    //operation->operate();
}

Operation* OperationList::undo() {
    Operation* operation = nullptr;
    if (curr - 1 == first) {
        return nullptr;
    }
    else if (curr > first) {
        operation = list[--curr];
    }
    else {
        if (curr == 0) {
            operation = list[MAX_SIZE - 1];
            curr = MAX_SIZE - 1;
        }
        else {
            operation = list[--curr];
        }
    }

    if (operation) {
        operation->undo();
        Env::map->setSelectedFeatures(operation->features);
    }
    return operation;
}

Operation* OperationList::redo() {
    Operation* operation = list[curr];
    if (!operation)
        return nullptr;

    curr = (curr+1) % MAX_SIZE;
    operation->operate();
    Env::map->setSelectedFeatures(operation->features);
    return operation;
}

void OperationList::clear() {
    for (int i = 0; i < MAX_SIZE; ++i) {
        if (list[i]) {
            delete list[i];
            list[i] = nullptr;
        }
    }
    first = 0;
    curr = 1;
    isOverrided = false;
}

Operation* OperationList::addDeleteOperation(const FeaturesList& features) {
    OperationDelete* op = new OperationDelete();
    op->features = features;
    addOperation(op);
    return op;
}

Operation* OperationList::addMoveOperation(const FeaturesList& features, double xOffset, double yOffset) {
    OperationMove* op = new OperationMove();
    op->features = features;
    op->xOffset = xOffset;
    op->yOffset = yOffset;
    addOperation(op);
    return op;
}

Operation* OperationList::addRotateOperation(const FeaturesList& features, double angle) {
    OperationRotate* op = new OperationRotate();
    op->features = features;
    op->angle = angle;
    for (auto& f1 : features) {
        for (auto& f2 : f1.second) {
            op->centrals.emplace(f2, f2->getExtent().center());
        }
    }
    addOperation(op);
    return op;
}
