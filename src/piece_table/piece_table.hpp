#pragma once
#include <vector>
#include <deque>
#include <string>
#include <memory>

class PieceTable
{
private:
    enum Color
    {
        RED,
        BLACK
    };
    struct BufferPosition
    {
        int index;
        int offset;

        BufferPosition(int index, int offset);
    };
    struct EditPiece
    {
        int bufferInfex;
        BufferPosition start;
        BufferPosition end;

        int leftSubTreeLength;
        int leftSubTreeLineCount;

        EditPiece(const int bufferInfex, const BufferPosition &start, const BufferPosition &end);
    };
    struct EditNode
    {
        EditPiece data;

        Color color;
        EditNode *parent;
        EditNode *left;
        EditNode *right;

        EditNode(EditPiece &data);
        ~EditNode();
    };
    struct Buffer
    {
        std::string str;
        int *lineStarts;
        int lineCount;
        
        Buffer(std::string str);
        Buffer(const Buffer &other);
        ~Buffer();

        Buffer &operator=(const Buffer &other);
    };

    EditNode *editTreeRoot;
    std::vector<Buffer> buffers;

    void change(const unsigned int &index, const unsigned int length, const std::string &data);
    unsigned int insertBuffer(std::string &data);
    void insertEdit(EditNode data);
    EditNode &findEdit(int &index);

public:
    PieceTable();
    ~PieceTable();

    PieceTable &insert(const unsigned int &index, const std::string &data);
    PieceTable &remove(const unsigned int &index, const unsigned int &length);
    PieceTable &replace(const unsigned int &index, const unsigned int &length, const std::string &data);

    std::string getLineContent();
};