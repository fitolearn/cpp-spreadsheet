#pragma once
#include "common.h"
#include "formula.h"
#include <functional>
#include <unordered_set>
#include <stack>
#include <set>
#include <optional>

class Sheet;
class Cell : public CellInterface {
public:
    explicit Cell(Sheet& sheet);
    ~Cell() override;

    void Set(std::string text);
    void Clear();

    [[nodiscard]] Value GetValue() const override;
    [[nodiscard]] std::string GetText() const override;
    [[nodiscard]] std::vector<Position> GetReferencedCells() const override;
    void CacheInvalidate(bool status = false);
private:
    class Impl {
    public:
        [[nodiscard]] virtual Value GetValue() const = 0;
        [[nodiscard]] virtual std::string GetText() const = 0;
        [[nodiscard]] virtual std::vector<Position> GetReferencedCells() const;
        virtual bool HasCache();
        virtual void ResetCache();
        virtual ~Impl() = default;
    };
    [[nodiscard]] bool CheckCircularDependencies(const Impl& new_impl) const;
    class EmptyImpl : public Impl {
    public:
        [[nodiscard]] Value GetValue() const override;
        [[nodiscard]] std::string GetText() const override;
    };

    class TextImpl : public Impl {
    public:
        explicit TextImpl(std::string text);
        [[nodiscard]] Value GetValue() const override;
        [[nodiscard]] std::string GetText() const override;
    private:
        std::string text_;
    };

    class FormulaImpl : public Impl {
    public:
        explicit FormulaImpl(const std::string& text, SheetInterface& sheet);
        Value GetValue() const override;
        std::string GetText() const override;
        std::vector<Position> GetReferencedCells() const override;
        bool HasCache() override;
        void ResetCache() override;
    private:
        mutable std::optional<FormulaInterface::Value> cache_;
        std::unique_ptr<FormulaInterface> formula_;
        SheetInterface& sheet_;
    };

    std::unique_ptr<Impl> impl_;
    Sheet& sheet_;
    std::set<Cell*> calculated_cells_;
    std::set<Cell*> used_cells_;
};
