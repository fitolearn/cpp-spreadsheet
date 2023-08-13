#include "cell.h"
#include "sheet.h"
#include <string>
#include <optional>

Cell::Cell(Sheet& sheet) : impl_(std::make_unique<EmptyImpl>()), sheet_(sheet) {}
Cell::~Cell() = default;

void Cell::Set(std::string text) {
    std::unique_ptr<Impl> impl;
    if (text.empty()) {
        impl = std::make_unique<EmptyImpl>();
    } else if (text.size() >= 2 && text[0] == FORMULA_SIGN) {
        impl = std::make_unique<FormulaImpl>(std::move(text), sheet_);
    } else {
        impl= std::make_unique<TextImpl>(std::move(text));
    }
    if (CheckCircularDependencies(*impl)) {
        throw CircularDependencyException("Circular dependency exception");
    }
    for (Cell* cell : used_cells_) {
        cell->calculated_cells_.erase(this);
    }
    used_cells_.clear();
    for (const auto& pos : impl_->GetReferencedCells()) {
        Cell* used = sheet_.GetCell(pos);
        if (!used){
            sheet_.SetCell(pos, "");
            used = sheet_.GetCell(pos);
        }
        used_cells_.insert(used);
        used ->calculated_cells_.insert(this);
    }
    impl_ = std::move(impl);
    CacheInvalidate();
}

bool Cell::CheckCircularDependencies(const Impl& new_impl) const {
    const auto& referenced_cells = new_impl.GetReferencedCells();
    if (!referenced_cells.empty()) {
        std::set<const Cell*> calculated, used;
        std::vector<const Cell*> progress;
        for (const auto& position : referenced_cells) {
            const Cell* ref_cell = sheet_.GetCell(position);
            if (ref_cell) {
                used.insert(ref_cell);
            } else {
                sheet_.SetCell(position, "");
                used.insert(sheet_.GetCell(position));
            }
        }
        progress.push_back(this);
        while (!progress.empty()) {
            const auto current = progress.back();
            if(current != nullptr){
                calculated.insert(current);
                for (const Cell* dependent : current->calculated_cells_) {
                    if (calculated.find(dependent) == calculated.end()) {
                        progress.push_back(dependent);
                    } else {
                        return true;
                    }
                } progress.pop_back();
            }
        }
    } return false;
}

void Cell::Clear() {
    impl_ = std::make_unique<EmptyImpl>();
}

Cell::Value Cell::GetValue() const {return impl_->GetValue();}
std::string Cell::GetText() const {return impl_->GetText();}
std::vector<Position> Cell::GetReferencedCells() const {return impl_->GetReferencedCells();}

void Cell::CacheInvalidate(bool status) {
    if (impl_->HasCache() || status) {
        impl_->ResetCache();

        for (Cell* dependent : calculated_cells_) {
            dependent->CacheInvalidate();
        }
    }
}

std::string Cell::FormulaImpl::GetText() const {
    return FORMULA_SIGN + formula_->GetExpression();
}
std::vector<Position> Cell::FormulaImpl::GetReferencedCells() const {
    return formula_->GetReferencedCells();
}
bool Cell::FormulaImpl::HasCache() {
    return cache_.has_value();
}
void Cell::FormulaImpl::ResetCache() {
    cache_.reset();
}

void Cell::Impl::ResetCache() {}
std::vector<Position> Cell::Impl::GetReferencedCells() const { return {};}
bool Cell::Impl::HasCache() { return true;}
Cell::Value Cell::EmptyImpl::GetValue() const { return "";}
std::string Cell::EmptyImpl::GetText() const { return "";}
Cell::TextImpl::TextImpl(std::string text) : text_(std::move(text)) {}

Cell::Value Cell::TextImpl::GetValue() const {
    if (text_.empty()) {
        throw FormulaException("Text expected");
    } else if (text_.at(0) == ESCAPE_SIGN) {
        return text_.substr(1);
    } else {
        return text_;
    }
}

std::string Cell::TextImpl::GetText() const { return text_;}

Cell::FormulaImpl::FormulaImpl(const std::string& text, SheetInterface& sheet) : formula_(ParseFormula(text.substr(1)))
        , sheet_(sheet) {}

Cell::Value Cell::FormulaImpl::GetValue() const {
    if (!cache_) {
        cache_ = formula_->Evaluate(sheet_);
    }
    return std::visit([](auto& helper){
        return Value(helper);
        }, *cache_);
}
