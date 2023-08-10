#pragma once

#include "common.h"
#include "formula.h"

class Cell : public CellInterface {
public:
    Cell();
    ~Cell();

    void Set(std::string text);
    void Clear();

    Value GetValue() const override;
    std::string GetText() const override;
// new
    std::vector<Position> GetReferencedCells() const override;
    void CacheInvalidate(); 
    bool IsCircularDependency(const Impl& impl) const;
private:
    class Impl;
    class EmptyImpl;
    class TextImpl;
    class FormulaImpl;
    std::unique_ptr<Impl> impl_;
    SheetInterface& sheet_;
};
