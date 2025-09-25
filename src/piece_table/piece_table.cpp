#include "piece_table.hpp"

PieceTable::EditNode::EditNode(EditPiece &data) : data(data), color(RED), parent(nullptr), left(nullptr), right(nullptr) {}
PieceTable::EditNode::EditNode(const EditPiece &data) : data(data), color(RED), parent(nullptr), left(nullptr), right(nullptr) {}

PieceTable::EditNode::~EditNode()
{
    delete right;
    delete left;
}

PieceTable::EditPiece::EditPiece(const size_t bufferInfex, const BufferPosition &start, const BufferPosition &end) : bufferInfex(bufferInfex), start(start), end(end), leftSubTreeLength(0), leftSubTreeLineCount(0) {}

PieceTable::BufferPosition::BufferPosition(size_t index, size_t offset) : index(index), offset(offset) {}

PieceTable::Buffer::Buffer(std::string str) : str(str), lineStarts(nullptr), lineCount(0)
{
    char temp;
    std::deque<size_t> linesList;
    linesList.push_back(0);

    for (size_t i = 0; i < str.length(); i++)
    {
        temp = str[i];
        if (temp == '\n')
        {
            linesList.push_back(i);
        }
    }
    this->lineStarts = new size_t[linesList.size()];
    while (!linesList.empty())
    {
        this->lineStarts[lineCount] = linesList.front();
        linesList.pop_front();
        lineCount++;
    }
};

PieceTable::Buffer::~Buffer()
{
    delete lineStarts;
}

PieceTable::Buffer::Buffer(const Buffer &other) : str(other.str), lineStarts(new size_t[other.lineCount]()), lineCount(other.lineCount)
{
    for (int i = 0; i < other.lineCount; i++)
    {
        this->lineStarts[i] = this->lineStarts[i];
    }
}
PieceTable::Buffer &PieceTable::Buffer::operator=(const Buffer &other)
{
    this->lineCount = other.lineCount;
    this->str = other.str;
    this->lineStarts = new size_t[other.lineCount]();
    for (int i = 0; i < other.lineCount; i++)
    {
        this->lineStarts[i] = this->lineStarts[i];
    }

    return *this;
}

PieceTable::PieceTable() : editTreeRoot(nullptr) {}

PieceTable::~PieceTable()
{
    delete editTreeRoot;
}

PieceTable &PieceTable::insert(const size_t index, const std::string &data)
{
    change(index, 0, data);
    return *this;
}

PieceTable &PieceTable::remove(const size_t index, const size_t &length)
{
    change(index, length, "");
    return *this;
}

PieceTable &PieceTable::replace(const size_t index, const size_t &length, const std::string &data)
{
    change(index, length, data);
    return *this;
}

void PieceTable::change(const size_t index, const size_t length, const std::string &data) {}

size_t PieceTable::insertBuffer(const std::string &data)
{
    this->buffers.emplace_back(data);
    return this->buffers.size() + size_t(-1);
}

size_t PieceTable::getEditPieceLength(const EditPiece &piece)
{
    size_t *const lineStarts = this->buffers[piece.bufferInfex].lineStarts;
    size_t startIndex = lineStarts[piece.start.index] + piece.start.offset;
    size_t endIndex = lineStarts[piece.end.index] + piece.end.offset;

    return endIndex - startIndex;
}

size_t PieceTable::getEditPieceLineCount(const EditPiece &piece)
{
    return piece.end.index - piece.start.index;
}

PieceTable::NodePosition::NodePosition(size_t nodeStartOffset, EditNode *node) : nodeStartOffset(nodeStartOffset), node(node) {}

PieceTable::NodePosition PieceTable::nodeAt(size_t index)
{
    EditNode *currentNode = editTreeRoot;
    size_t currentOffset = 0;
    while (currentNode != nullptr)
    {
        if (currentNode->data.leftSubTreeLength > index)
        {
            currentNode = currentNode->left;
        }
        else if (currentNode->data.leftSubTreeLength + getEditPieceLength(currentNode->data) >= index)
        {
            return NodePosition(currentOffset, currentNode);
        }
        else
        {
            index -= currentNode->data.leftSubTreeLength + getEditPieceLength(currentNode->data);
            currentOffset += currentNode->data.leftSubTreeLength + getEditPieceLength(currentNode->data);
            currentNode = currentNode->right;
        }
    }

    return NodePosition(0, nullptr);
}

PieceTable::EditNode *PieceTable::findSmallest(EditNode *node)
{
    if (node == nullptr)
        return nullptr;
    while (node->left != nullptr)
    {
        node = node->left;
    }
    return node;
}

PieceTable::EditNode *PieceTable::findBiggest(EditNode *node)
{
    if (node == nullptr)
        return nullptr;
    while (node->right != nullptr)
    {
        node = node->right;
    }
    return node;
}

/**
 *      node              node
 *     /  \              /  \
 *    a   b    ---->   a    b
 *                         /
 *                        z
 */
PieceTable::EditNode *PieceTable::insertRight(EditNode *const node, const EditPiece &piece)
{
    EditNode *newNode = new EditNode(piece);
    newNode->data.leftSubTreeLength = 0;
    newNode->data.leftSubTreeLineCount = 0;

    if (editTreeRoot == nullptr)
    {
        editTreeRoot = newNode;
        editTreeRoot->color = BLACK;
    }
    else if (node->right == nullptr)
    {
        node->right = newNode;
        newNode->parent = node;
    }
    else
    {
        EditNode *nextNode = findSmallest(node);
        nextNode->left = newNode;
        newNode->parent = nextNode;
    }

    fixInsert(newNode);

    return newNode;
}

/**
 *      node              node
 *     /  \              /  \
 *    a   b     ---->   a    b
 *                       \
 *                        z
 */
PieceTable::EditNode *PieceTable::insertLeft(EditNode *const node, const EditPiece &piece)
{
    EditNode *newNode = new EditNode(piece);
    newNode->data.leftSubTreeLength = 0;
    newNode->data.leftSubTreeLineCount = 0;

    if (editTreeRoot == nullptr)
    {
        editTreeRoot = newNode;
        editTreeRoot->color = BLACK;
    }
    else if (node->left == nullptr)
    {
        node->left = newNode;
        newNode->parent = node;
    }
    else
    {
        EditNode *nextNode = findBiggest(node);
        nextNode->right = newNode;
        newNode->parent = nextNode;
    }

    fixInsert(newNode);

    return newNode;
}

void PieceTable::rotateRight(EditNode *node)
{
    EditNode *child = node->left;

    // fix size of parent
    node->data.leftSubTreeLength -= (child->data.leftSubTreeLength + getEditPieceLength(child->data));
    node->data.leftSubTreeLineCount -= (child->data.leftSubTreeLineCount + getEditPieceLineCount(child->data));

    node->left = child->right;
    if (node->left != nullptr)
        node->left->parent = node;
    child->parent = node->parent;
    if (node->parent == nullptr)
        editTreeRoot = child;
    else if (node == node->parent->left)
        node->parent->left = child;
    else
        node->parent->right = child;
    child->right = node;
    node->parent = child;
}

void PieceTable::rotateLeft(EditNode *node)
{
    EditNode *child = node->right;

    // fix size of child
    child->data.leftSubTreeLength += node->data.leftSubTreeLength + getEditPieceLength(node->data);
    child->data.leftSubTreeLineCount += node->data.leftSubTreeLineCount + getEditPieceLineCount(node->data);

    node->right = child->left;
    if (node->right != nullptr)
        node->right->parent = node;
    child->parent = node->parent;
    if (node->parent == nullptr)
        editTreeRoot = child;
    else if (node == node->parent->left)
        node->parent->left = child;
    else
        node->parent->right = child;
    child->left = node;
    node->parent = child;
}

void PieceTable::fixInsert(EditNode *node)
{
    updateMetadata(node);

    EditNode *parent = nullptr;
    EditNode *grandparent = nullptr;
    while (node != editTreeRoot && node->color == RED && node->parent->color == RED)
    {
        parent = node->parent;
        grandparent = parent->parent;
        if (parent == grandparent->left)
        {
            EditNode *uncle = grandparent->right;
            if (uncle != nullptr && uncle->color == RED)
            {
                grandparent->color = RED;
                parent->color = BLACK;
                uncle->color = BLACK;
                node = grandparent;
            }
            else
            {
                if (node == parent->right)
                {
                    rotateLeft(parent);
                    node = parent;
                    parent = node->parent;
                }
                rotateRight(grandparent);
                std::swap(parent->color, grandparent->color);
                node = parent;
            }
        }
        else
        {
            EditNode *uncle = grandparent->left;
            if (uncle != nullptr && uncle->color == RED)
            {
                grandparent->color = RED;
                parent->color = BLACK;
                uncle->color = BLACK;
                node = grandparent;
            }
            else
            {
                if (node == parent->left)
                {
                    rotateRight(parent);
                    node = parent;
                    parent = node->parent;
                }
                rotateLeft(grandparent);
                std::swap(parent->color, grandparent->color);
                node = parent;
            }
        }
    }
    editTreeRoot->color = BLACK;
}

void PieceTable::updateMetadata(EditNode *node)
{
    if (node == editTreeRoot)
        return;

    // go upwards till the node whose left subtree is changed.
    while (node != editTreeRoot && node == node->parent->right)
    {
        node = node->parent;
    }

    if (node == editTreeRoot)
        // well, it means we add a node to the end (inorder)
        return;

    node = node->parent;

    size_t lengthDelta = calculateLength(node->left) - node->data.leftSubTreeLength;
    size_t lineCountDelta = calculateLineCount(node->left) - node->data.leftSubTreeLineCount;

    node->data.leftSubTreeLength += lengthDelta;
    node->data.leftSubTreeLineCount += lineCountDelta;

    while (node != editTreeRoot && (lengthDelta != 0 || lineCountDelta != 0))
    {
        if (node->parent->left == node)
        {
            node->parent->data.leftSubTreeLength += lengthDelta;
            node->parent->data.leftSubTreeLineCount += lineCountDelta;
        }
        node = node->parent;
    }
}

size_t PieceTable::calculateLength(EditNode *node)
{
    if (node == nullptr)
    {
        return 0;
    }

    return node->data.leftSubTreeLength + getEditPieceLength(node->data) + calculateLength(node->right);
}

size_t PieceTable::calculateLineCount(EditNode *node)
{
    if (node == nullptr)
    {
        return 0;
    }

    return node->data.leftSubTreeLineCount + getEditPieceLineCount(node->data) + calculateLineCount(node->right);
}