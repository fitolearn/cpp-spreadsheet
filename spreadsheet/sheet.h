#pragma once
#include "cell.h"
#include "common.h"
#include <unordered_map>
#include <functional>

class Sheet : public SheetInterface {
public:
    ~Sheet() override;
    void SetCell(Position pos, std::string text) override;
    const Cell* GetCell(Position pos) const override;
    Cell* GetCell(Position pos) override;
    void ClearCell(Position pos) override;
    Size GetPrintableSize() const override;
    void PrintValues(std::ostream& output) const override;
    void PrintTexts(std::ostream& output) const override;
    std::pair<Position, Position> GetUseableArea() const;
private:
    struct Hasher {
        size_t operator()(const Position& pos) const;
    };
    std::unordered_map<Position, std::unique_ptr<Cell>, Hasher> cells_;
};
