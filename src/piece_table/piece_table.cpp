#include "piece_table.hpp"

PieceTable::EditNode::EditNode(EditPiece &data) : data(data), color(RED), parent(nullptr), left(nullptr), right(nullptr) {}
PieceTable::EditNode::EditNode(const EditPiece &data) : data(data), color(RED), parent(nullptr), left(nullptr), right(nullptr) {}

PieceTable::EditNode::~EditNode()
{
    delete right;
    delete left;
}

PieceTable::EditNode &PieceTable::EditNode::operator=(const EditNode &other)
{
    if (this != &other)
    {
        this->color = other.color;
        this->data = other.data;
        this->left = other.left;
        this->parent = other.parent;
        this->right = other.right;
    }

    return *this;
}

PieceTable::EditPiece::EditPiece(const size_t bufferInfex, const BufferPosition &start, const BufferPosition &end) : bufferInfex(bufferInfex), start(start), end(end), leftSubTreeLength(0), leftSubTreeLineCount(0) {}

PieceTable::EditPiece::EditPiece(const EditPiece &other) : bufferInfex(other.bufferInfex), end(other.end), leftSubTreeLength(other.leftSubTreeLength), leftSubTreeLineCount(other.leftSubTreeLineCount), start(other.start) {}

PieceTable::EditPiece &PieceTable::EditPiece::operator=(const EditPiece &other)
{
    if (this != &other)
    {
        this->bufferInfex = other.bufferInfex;
        this->end = other.end;
        this->leftSubTreeLength = other.leftSubTreeLength;
        this->leftSubTreeLineCount = other.leftSubTreeLineCount;
        this->start = other.start;
    }

    return *this;
}

PieceTable::BufferPosition::BufferPosition(size_t index, size_t offset) : index(index), offset(offset) {}

PieceTable::BufferPosition::BufferPosition(const BufferPosition &other) : index(other.index), offset(other.offset) {}

PieceTable::BufferPosition &PieceTable::BufferPosition::operator=(const BufferPosition &other)
{
    this->index = other.index;
    this->offset = other.offset;

    return *this;
}

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
        this->lineStarts[i] = other.lineStarts[i];
    }
}
PieceTable::Buffer &PieceTable::Buffer::operator=(const Buffer &other)
{
    if (this != &other)
    {
        this->lineCount = other.lineCount;
        this->str = other.str;
        this->lineStarts = new size_t[other.lineCount]();
        for (int i = 0; i < other.lineCount; i++)
        {
            this->lineStarts[i] = other.lineStarts[i];
        }
    }

    return *this;
}

PieceTable::NodeArrayStruct::NodeArrayStruct() : pieces(nullptr), size(0) {}
PieceTable::NodeArrayStruct::NodeArrayStruct(size_t size) : pieces(new EditPiece[size]()), size(size) {}
PieceTable::NodeArrayStruct::NodeArrayStruct(const NodeArrayStruct &other) : pieces(new EditPiece[other.size]()), size(other.size)
{
    for (size_t i = 0; i < other.size; i++)
    {
        this->pieces[i] = other.pieces[i];
    }
}
PieceTable::NodeArrayStruct::~NodeArrayStruct()
{
    delete[] pieces;
}

PieceTable::PieceTable() : editTreeRoot(nullptr) {}

PieceTable::~PieceTable()
{
    delete editTreeRoot;
}

PieceTable &PieceTable::insert(const size_t index, const std::string &data)
{
    // this needs to be enum
    // we need to know the exact number of objects stored in the array
    NodeArrayStruct x = createPieces(data);

    if (editTreeRoot == nullptr)
    {
        // tree is empty
        EditNode *node = insertRight(nullptr, x.pieces[0]);
        for (size_t i = 1; i < x.size; i++)
        {
            node = insertRight(node, x.pieces[i]);
        }
        return *this;
    }
    else
    {
        // tree is not empty
        NodePosition nodePosition = nodeAt(index);

        if (nodePosition.nodeStartOffset == index)
        {
            // we are inserting into the beginning of a node.
            // we insert one node to left and then all the rest of the nodes in sequanse to the right
            EditNode *node = insertLeft(nodePosition.node, x.pieces[0]);
            for (size_t i = 1; i < x.size; i++)
            {
                node = insertRight(node, x.pieces[i]);
            }
        }
        else if (nodePosition.nodeStartOffset + getEditPieceLength(nodePosition.node->data) > index)
        {
            // we are inserting into the middle of a node.
            size_t offsetInNode = index - nodePosition.nodeStartOffset;
            splitNode(nodePosition.node, offsetInNode);

            EditNode *node = insertRight(nodePosition.node, x.pieces[0]);
            for (size_t i = 1; i < x.size; i++)
            {
                node = insertRight(node, x.pieces[i]);
            }
        }
        else
        {
            // we are inserting into the end of a node.
            // we insert all nodes in sequanse to the right
            EditNode *node = insertRight(nodePosition.node, x.pieces[0]);
            for (size_t i = 1; i < x.size; i++)
            {
                node = insertRight(node, x.pieces[i]);
            }
        }
    }

    return *this;
}

void PieceTable::splitNode(EditNode *const node, size_t offset)
{
    EditPiece &piece = node->data;
    size_t bufferIndex = piece.bufferInfex;
    size_t splitOffset = 0, lineStart = 0;
    for (size_t i = 0; i < this->buffers[bufferIndex].lineCount; i++)
    {
        size_t currOffsetOfLine = this->buffers[bufferIndex].lineStarts[i];
        if (currOffsetOfLine > offset)
        {
            break;
        }
        lineStart = i;
        splitOffset = offset - currOffsetOfLine;
    }

    BufferPosition splitPoint = BufferPosition(lineStart, splitOffset);
    BufferPosition newEnd = BufferPosition(piece.end);
    piece.end = splitPoint;

    EditPiece newNode = EditPiece(bufferIndex, splitPoint, newEnd);
    insertRight(node, newNode);
}

PieceTable::NodeArrayStruct PieceTable::createPieces(const std::string &data)
{
    size_t data_length = data.size();
    // padding the int so the integer devision will be roung up
    size_t arr_len = (data_length + MAX_CHAR_PER_NODE - 1) / MAX_CHAR_PER_NODE;

    NodeArrayStruct retData(arr_len);

    size_t index = 0;
    size_t lengthUsed = 0;
    std::string subString;
    size_t charsToUse, bufferIndex, lastLineIndex, strLen;
    BufferPosition start, end;
    while (lengthUsed != data_length)
    {
        if (lengthUsed + MAX_CHAR_PER_NODE < data_length)
        {
            charsToUse = MAX_CHAR_PER_NODE;
        }
        else
        {
            charsToUse = data_length - lengthUsed;
        }

        subString = data.substr(lengthUsed, charsToUse);

        bufferIndex = insertBuffer(subString);
        start = BufferPosition(0, 0);
        lastLineIndex = buffers[bufferIndex].lineCount - 1;
        strLen = buffers[bufferIndex].str.size();
        end = BufferPosition(lastLineIndex, strLen - buffers[bufferIndex].lineStarts[lastLineIndex]);
        retData.pieces[index] = EditPiece(bufferIndex, start, end);

        lengthUsed += charsToUse;
        index++;
    }

    return retData;
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
        EditNode *nextNode = findSmallest(node->right);
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
        EditNode *nextNode = findBiggest(node->left);
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

std::string PieceTable::getLineContent(size_t line)
{
    EditNode *currentNode = editTreeRoot;
    while (currentNode != nullptr)
    {
        if (currentNode->left && currentNode->data.leftSubTreeLineCount >= line)
        {
            currentNode = currentNode->left;
        }
        else if (currentNode->data.leftSubTreeLineCount + getEditPieceLineCount(currentNode->data) > line)
        {
            // the hole line is in this node
            line -= currentNode->data.leftSubTreeLineCount;
            size_t startIndex = buffers[currentNode->data.bufferInfex].lineStarts[line] + currentNode->data.start.offset;
            size_t endIndex = buffers[currentNode->data.bufferInfex].lineStarts[line + 1];
            return buffers[currentNode->data.bufferInfex].str.substr(startIndex, endIndex - startIndex);
        }
        else if (currentNode->data.leftSubTreeLineCount + getEditPieceLineCount(currentNode->data) == line)
        {
            // the node is the begining of the line but may not be the last
            std::string tempString, retString = "";
            line -= currentNode->data.leftSubTreeLineCount;
            size_t startIndex = buffers[currentNode->data.bufferInfex].lineStarts[line] + currentNode->data.start.offset;
            size_t endIndex = buffers[currentNode->data.bufferInfex].str.size() - 1;
            tempString = buffers[currentNode->data.bufferInfex].str.substr(startIndex, endIndex - startIndex);

            EditNode *nextNode = getNextNode(currentNode);
            size_t nextNodeLineCount;

            while (nextNode)
            {
                nextNodeLineCount = getEditPieceLineCount(nextNode->data);
                // there are no line breakes in the piece and we can take the hole piece
                tempString += getEditPieceTextTillEndLine(nextNode->data);
                if (nextNodeLineCount != 0)
                {
                    break;
                }

                nextNode = getNextNode(currentNode);
            }

            return tempString;
        }
        else
        {
            line -= currentNode->data.leftSubTreeLength + getEditPieceLineCount(currentNode->data);
            currentNode = currentNode->right;
        }
    }
    return "TBH this is not supposed to come to here :) yet here we are";
}

PieceTable::EditNode *PieceTable::getNextNode(EditNode *node)
{
    if (node == nullptr)
    {
        return nullptr;
    }

    if (node->right)
    {
        return findSmallest(node->right);
    }

    EditNode *parent = node->parent;
    while (parent && node == parent->right)
    {
        node = parent;
        parent = parent->parent;
    }

    return node->parent;
}

std::string PieceTable::getEditPieceText(EditPiece &piece)
{
    Buffer &currentBuffer = buffers[piece.bufferInfex];
    size_t startIndex = currentBuffer.lineStarts[piece.start.index] + piece.start.offset;
    size_t endIndex = currentBuffer.lineStarts[piece.end.index] + piece.end.offset;
    size_t strLen = endIndex - startIndex;

    return currentBuffer.str.substr(startIndex, strLen);
}

std::string PieceTable::getEditPieceTextTillEndLine(EditPiece &piece)
{
    if (getEditPieceLineCount(piece) == 0)
    {
        return getEditPieceText(piece);
    }

    Buffer &currentBuffer = buffers[piece.bufferInfex];
    size_t startIndex = currentBuffer.lineStarts[piece.start.index] + piece.start.offset;
    size_t endIndex = currentBuffer.lineStarts[piece.start.index + 1]; // we know for sure that this exist since getEditPieceLineCount returned > 0
    size_t strLen = endIndex - startIndex;

    return currentBuffer.str.substr(startIndex, strLen);
}