#include "piece_table.hpp"

PieceTable::EditNode::EditNode(EditPiece &data) : data(data), color(RED), parent(nullptr), left(nullptr), right(nullptr) {}

PieceTable::EditNode::~EditNode()
{
    delete right;
    delete left;
}

PieceTable::EditPiece::EditPiece(const int bufferInfex, const BufferPosition &start, const BufferPosition &end) : bufferInfex(bufferInfex), start(start), end(end) {}

PieceTable::BufferPosition::BufferPosition(int index, int offset) : index(index), offset(offset) {}

PieceTable::Buffer::Buffer(std::string str) : str(str), lineStarts(nullptr)
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
    int lineCount = 0;
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

int PieceTable::insertBuffer(std::string &data)
{
    this->buffers.push_back(std::make_unique<Buffer>(data));
    return this->buffers.size() - 1;
}