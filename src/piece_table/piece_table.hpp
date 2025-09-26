#pragma once
#include <vector>
#include <deque>
#include <string>
#include <memory>

class PieceTable
{
private:
    const size_t MAX_CHAR_PER_NODE = 200;
    enum Color
    {
        RED,
        BLACK
    };
    struct BufferPosition
    {
        size_t index;
        size_t offset;

        BufferPosition() = default;
        BufferPosition(const BufferPosition& other);
        BufferPosition(size_t index, size_t offset);
        BufferPosition &operator=(const BufferPosition &other);
    };
    struct EditPiece
    {
        size_t bufferInfex;
        BufferPosition start;
        BufferPosition end;

        size_t leftSubTreeLength;
        size_t leftSubTreeLineCount;

        EditPiece() = default;
        EditPiece(const EditPiece &other);
        EditPiece &operator=(const EditPiece &other);
        EditPiece(const size_t bufferInfex, const BufferPosition &start, const BufferPosition &end);
    };
    struct EditNode
    {
        EditPiece data;

        Color color;
        EditNode *parent;
        EditNode *left;
        EditNode *right;

        EditNode(EditPiece &data);
        EditNode(const EditPiece &data);
        EditNode &operator=(const EditNode &other);
        ~EditNode();
    };
    struct Buffer
    {
        std::string str;
        size_t *lineStarts;
        size_t lineCount;

        Buffer(std::string str);
        Buffer(const Buffer &other);
        ~Buffer();

        Buffer &operator=(const Buffer &other);
    };
    struct NodePosition
    {
        size_t nodeStartOffset;
        EditNode *node;

        NodePosition(size_t nodeStartOffset, EditNode *node);
    };
    struct NodeArrayStruct
    {
        EditPiece *pieces;
        size_t size;
        NodeArrayStruct();
        NodeArrayStruct(NodeArrayStruct &other);
        NodeArrayStruct(size_t size);
        ~NodeArrayStruct();
    };

    EditNode *editTreeRoot;
    std::vector<Buffer> buffers;

    void change(const size_t index, const size_t length, const std::string &data);
    size_t insertBuffer(const std::string &data);
    // void insertEdit(EditNode data);
    NodePosition nodeAt(size_t index);
    size_t getEditPieceLength(const EditPiece &piece);
    size_t getEditPieceLineCount(const EditPiece &piece);
    EditNode *insertRight(EditNode *const node, const EditPiece &piece);
    EditNode *insertLeft(EditNode *const node, const EditPiece &piece);
    EditNode *findSmallest(EditNode *node);
    EditNode *findBiggest(EditNode *node);
    void fixInsert(EditNode *node);
    void updateMetadata(EditNode *node);
    void rotateRight(EditNode *node);
    void rotateLeft(EditNode *node);
    size_t calculateLength(EditNode *node);
    size_t calculateLineCount(EditNode *node);
    NodeArrayStruct createPieces(const std::string &data);
    void splitNode(EditNode *const node,size_t offset);

public:
    PieceTable();
    ~PieceTable();

    PieceTable &insert(const size_t index, const std::string &data);
    PieceTable &remove(const size_t index, const size_t &length);
    PieceTable &replace(const size_t index, const size_t &length, const std::string &data);

    // std::string getLineContent();
};