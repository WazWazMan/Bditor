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