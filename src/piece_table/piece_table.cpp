#include "piece_table.hpp"

PieceTable::EditNode::EditNode(EditPiece &data) : data(data), color(RED), parent(nullptr), left(nullptr), right(nullptr) {}
PieceTable::EditNode::EditNode(const EditPiece &data) : data(data), color(RED), parent(nullptr), left(nullptr), right(nullptr) {}

PieceTable::EditNode::~EditNode()
{
    delete right;
    delete left;
}

PieceTable::EditPiece::EditPiece(const int bufferInfex, const BufferPosition &start, const BufferPosition &end) : bufferInfex(bufferInfex), start(start), end(end), leftSubTreeLength(0), leftSubTreeLineCount(0) {}

PieceTable::BufferPosition::BufferPosition(int index, int offset) : index(index), offset(offset) {}

PieceTable::Buffer::Buffer(std::string str) : str(str), lineStarts(nullptr), lineCount(0)
{
    char temp;
    std::deque<int> linesList;
    linesList.push_back(0);

    for (unsigned int i = 0; i < str.length(); i++)
    {
        temp = str[i];
        if (temp == '\n')
        {
            linesList.push_back(i);
        }
    }
    this->lineStarts = new int[linesList.size()];
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

PieceTable::Buffer::Buffer(const Buffer &other) : str(other.str), lineStarts(new int[other.lineCount]()), lineCount(other.lineCount)
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
    this->lineStarts = new int[other.lineCount]();
    for (int i = 0; i < other.lineCount; i++)
    {
        this->lineStarts[i] = this->lineStarts[i];
    }

    return *this;
}

PieceTable::PieceTable() : editTreeRoot(nullptr)
{
}

PieceTable::~PieceTable()
{
    delete editTreeRoot;
}

PieceTable &PieceTable::insert(const unsigned int &index, const std::string &data)
{
    change(index, 0, data);
    return *this;
}

PieceTable &PieceTable::remove(const unsigned int &index, const unsigned int &length)
{
    change(index, length, "");
    return *this;
}

PieceTable &PieceTable::replace(const unsigned int &index, const unsigned int &length, const std::string &data)
{
    change(index, length, data);
    return *this;
}

void PieceTable::change(const unsigned int &index, const unsigned int length, const std::string &data) {}

size_t PieceTable::insertBuffer(std::string &data)
{
    this->buffers.emplace_back(data);
    return this->buffers.size() + size_t(-1);
}

unsigned int PieceTable::getEditPieceLength(const EditPiece &piece)
{
    int *const lineStarts = this->buffers[piece.bufferInfex].lineStarts;
    int startIndex = lineStarts[piece.start.index] + piece.start.offset;
    int endIndex = lineStarts[piece.end.index] + piece.end.offset;

    return endIndex - startIndex;
}

unsigned int PieceTable::getEditPieceLineCount(const EditPiece &piece)
{
    return piece.end.index - piece.start.index;
}

PieceTable::NodePosition::NodePosition(int nodeStartOffset, EditNode *node) : nodeStartOffset(nodeStartOffset), node(node) {}

PieceTable::NodePosition PieceTable::nodeAt(int index)
{
    EditNode *currentNode = editTreeRoot;
    int currentOffset = 0;
    while (currentNode != nullptr)
    {
        if (currentNode->data.leftSubTreeLength > index)
        {
            currentNode = currentNode->left;
        }
        else if (currentNode->data.leftSubTreeLength + getEditPieceLength(currentNode->data) > index)
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
void PieceTable::insertRight(EditNode *const node, const EditPiece &piece)
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
}

/**
 *      node              node
 *     /  \              /  \
 *    a   b     ---->   a    b
 *                       \
 *                        z
 */
void PieceTable::insertLeft(EditNode *const node, const EditPiece &piece)
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

    int lengthDelta = calculateLength(node->left) - node->data.leftSubTreeLength;
    int lineCountDelta = calculateLineCount(node->left) - node->data.leftSubTreeLineCount;

    node->data.leftSubTreeLength += lengthDelta;
    node->data.leftSubTreeLineCount += lineCountDelta;

    while (node != editTreeRoot && (lengthDelta != 0 || lineCountDelta != 0))
    {
        if(node->parent->left == node){
            node->parent->data.leftSubTreeLength += lengthDelta;
            node->parent->data.leftSubTreeLineCount += lineCountDelta;
        }
        node = node->parent;
    }
}

unsigned int PieceTable::calculateLength(EditNode *node){
    if(node == nullptr){
        return 0;
    }

    return node->data.leftSubTreeLength + getEditPieceLength(node->data) + calculateLength(node->right);
}

unsigned int PieceTable::calculateLineCount(EditNode *node){
    if(node == nullptr){
        return 0;
    }

    return node->data.leftSubTreeLineCount + getEditPieceLineCount(node->data) + calculateLineCount(node->right);
}
