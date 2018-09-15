#include "mgnode.h"


/**
 * Set this node's parent.
 */
void MgNode::setParent(MgNode *parent)
{
    // Make connection.
    parent->addChild(this);
}

/**
 * Add node to children list.
 */
void MgNode::addChild(MgNode *child)
{
    // If already his child, return.
    if (children.contains(child))
    {
        return;
    }

    // If child already has parent, remove him from his list.
    if (child->parent != nullptr)
    {
        child->parent->removeChild(child);
    }

    // Change the child's parent.
    child->parent = this;

    // Add the child to the list.
    children.append(child);
}

/**
 * Remove this node's parent.
 */
void MgNode::removeParent()
{
    // Remove connection.
    if (parent != nullptr)
    {
        parent->removeChild(this);
    }
}

/**
 * Remove node from children list.
 */
void MgNode::removeChild(MgNode *child)
{
    // If child is this parent's, remove him.
    int childIdx = children.indexOf(child);

    if (childIdx != -1)
    {
        child->parent = nullptr;
        children.remove(childIdx);
    }
}

/**
 * Compute matrix based on ancestry.
 */
void MgNode::computeMatrix()
{
    // If this has parent, get his matrix.
    QMatrix4x4 parentMatrix;

    if (parent != nullptr)
    {
        parentMatrix = parent->matrix;
    }

    // Compute the matrix.
    matrix = parentMatrix * matrix;
}

void MgNode::getMatrix(QMatrix4x4 *matrix)
{

}
