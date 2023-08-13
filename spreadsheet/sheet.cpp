#include "sheet.h"
#include "cell.h"
#include "common.h"
#include <algorithm>
#include <iostream>
#include <optional>

using namespace std::literals;

Sheet::~Sheet() = default;

void Sheet::SetCell(Position pos, std::string text) {
    if(!pos.IsValid()) {
        throw InvalidPositionException("Invalid position exception.");
    }
    bool isFound = text.substr(1).find(pos.ToString()) != std::string::npos;
    if(isFound) {
        throw CircularDependencyException("Circular dependency exception.");
    }
    if (!cells_.count(pos)) {
        cells_.emplace(pos, std::make_unique<Cell>(*this));
    }
    cells_.at(pos)->Set(std::move(text));
}

const Cell* Sheet::GetCell(Position pos) const {
    if(!pos.IsValid()) {
        throw InvalidPositionException("Invalid position exception.");
    }
    return cells_.count(pos) ? cells_.at(pos).get() : nullptr;
}

Cell* Sheet::GetCell(Position pos) {
    if(!pos.IsValid()) {
        throw InvalidPositionException("Invalid position exception.");
    }
    return cells_.count(pos) ? cells_.at(pos).get() : nullptr;
}

void Sheet::ClearCell(Position pos) {
    if(!pos.IsValid()) {
        throw InvalidPositionException("Invalid position exception.");
    }
    cells_.erase(pos);
}

Size Sheet::GetPrintableSize() const {
    if(cells_.empty()) {
        return Size{0, 0};
    }
    auto [left_top_point, right_bottom_point] = GetUseableArea();
    return Size{right_bottom_point.row - left_top_point.row + 1, right_bottom_point.col - left_top_point.col + 1};
}

void Sheet::PrintValues(std::ostream& output) const {
    if(cells_.empty()) {
        return;
    }
    auto [left_top_point, right_bottom_point] = GetUseableArea();
    for(int row = left_top_point.row; row <= right_bottom_point.row; ++row) {
        for(int col = left_top_point.col; col <= right_bottom_point.col; ++col) {
            if(col > 0) {
                output << '\t';
            }
            auto cell = GetCell(Position{row, col});
            if(cell != nullptr) {
                std::visit([&output](const auto value) {
                    output << value;
                }, cell->GetValue());
            }
        } output << '\n';
    }
}

void Sheet::PrintTexts(std::ostream& output) const {
    if(cells_.empty()) {
        return;
    }
    auto [left_top_point, right_bottom_point] = GetUseableArea();
    for(int row = left_top_point.row; row <= right_bottom_point.row; ++row) {
        for(int col = left_top_point.col; col <= right_bottom_point.col; ++col) {
            if(col > 0) {
                output << '\t';
            }
            auto cell = GetCell(Position{row, col});
            if(cell != nullptr) {
                output << cell->GetText();
            }
        } output << '\n';
    }
}

std::pair<Position, Position> Sheet::GetUseableArea() const {
    Position LeftTopPos, RightBottomPos;
    if(!cells_.empty()) {
        for (const auto& [pos, cell] : cells_) {
            if (cell != nullptr) {
                RightBottomPos.row = std::max(RightBottomPos.row, pos.row);
                RightBottomPos.col = std::max(RightBottomPos.col, pos.col);
            }
        }
    }
    return std::make_pair(LeftTopPos, RightBottomPos);
}

size_t Sheet::Hasher::operator() (const Position& pos) const {
    return std::hash<int>()(pos.row) * 37 + std::hash<int>()(pos.col) * 37 * 37;
}

std::unique_ptr<SheetInterface> CreateSheet() {
    return std::make_unique<Sheet>();
}
