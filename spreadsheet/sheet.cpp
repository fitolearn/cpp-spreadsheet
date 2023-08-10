#include "sheet.h"

#include "cell.h"
#include "common.h"

#include <algorithm>
#include <functional>
#include <iostream>
#include <optional>

using namespace std::literals;

Sheet::~Sheet() = default;

void Sheet::SetCell(Position pos, std::string text) {
    if (pos.IsValid()){
        auto sheet_pos = pos.ToString();
        cells_.resize(std::max(pos.row + 1, int(std::size(cells_))));
        cells_[pos.row].resize(std::max(pos.col + 1, int(std::size(cells_[pos.row]))));

        if (!cells_[pos.row][pos.col]){
            cells_[pos.row][pos.col] = std::make_unique<Cell>();
        }

        if (std::string text_tmp = text; !text_tmp.empty() && text_tmp[0] == FORMULA_SIGN){
            text_tmp = text_tmp.substr(1);
            Position tmp_pos = Position::FromString(text_tmp);
            if (tmp_pos.IsValid() && !Sheet::GetCell(tmp_pos)){
                Sheet::SetCell(tmp_pos, "");
            }
        }
        cells_[pos.row][pos.col]->Set(std::move(text));
    } else {
        throw InvalidPositionException("Position exception. SetCell function.");
    }
}

CellInterface* Sheet::GetCell(Position pos) {
    if (pos.IsValid()){
        if (size_t(pos.row) < std::size(cells_) && size_t(pos.col) < std::size(cells_[pos.row])){
            if (cells_[pos.row][pos.col].get() == nullptr){
                return nullptr;
            }
            return cells_[pos.row][pos.col].get();
        } else {
            return nullptr;
        }
    } else {
        throw InvalidPositionException("Position exception. GetCell function");
    }
}

const CellInterface* Sheet::GetCell(Position pos) const {
    if (pos.IsValid()){
        if (size_t(pos.row) < std::size(cells_) && size_t(pos.col) < std::size(cells_[pos.row])){
            if (cells_[pos.row][pos.col].get() == nullptr){
                return nullptr;
            }
            return cells_[pos.row][pos.col].get();
        } else {
            return nullptr;
        }
    } else {
        throw InvalidPositionException("Position exception. GetCell const function.");
    }
}

void Sheet::ClearCell(Position pos) {
    if (pos.IsValid()){
        if (size_t(pos.row) < std::size(cells_) && size_t(pos.col) < std::size(cells_[pos.row])){
            if (cells_[pos.row][pos.col]){
                cells_[pos.row][pos.col]->Clear();
            }
        }
    } else {
        throw InvalidPositionException("Position exception. ClearCell function.");
    }
}

Size Sheet::GetPrintableSize() const {
    Size size;
    for (int row = 0; row < int(std::size(cells_)); ++row){
        for (int col = int(std::size(cells_[row])) - 1; col >= 0; --col){
            if (cells_[row][col]){
                if (!cells_[row][col]->GetText().empty()){
                    size.rows = std::max(size.rows, row + 1);
                    size.cols = std::max(size.cols, col + 1);
                    break;
                }
            }
        }
    } return size;
}

void Sheet::PrintValues(std::ostream& output) const {
    for (int row = 0; row < GetPrintableSize().rows; ++row){
        for (int col = 0; col < GetPrintableSize().cols; ++col){
            if (col > 0){
                output << '\t';
            }
            if (col < int(std::size(cells_[row]))){
                if (cells_[row][col]){
                    std::visit([&output](const auto& value){
                        output << value;
                    }, cells_[row][col]->GetValue());}
            }
        } output << '\n';
    }
}

void Sheet::PrintTexts(std::ostream& output) const {
    for (int row = 0; row < GetPrintableSize().rows; ++row){
        for (size_t col = 0; col < size_t(GetPrintableSize().cols); ++col){
            if (col){
                output << '\t';
            }
            if (col < std::size(cells_[row])){
                if (cells_[row][col]) {
                    output << cells_[row][col]->GetText();
                }
            }
        } output << '\n';
    }
}

std::unique_ptr<SheetInterface> CreateSheet() {
    return std::make_unique<Sheet>();
}
