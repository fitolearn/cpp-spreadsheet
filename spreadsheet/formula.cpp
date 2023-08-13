#include "formula.h"
#include "FormulaAST.h"
#include <sstream>

using namespace std::literals;

std::ostream& operator<<(std::ostream& output, FormulaError fe) {
    return output << "#DIV/0!";
}

namespace {
    class Formula : public FormulaInterface {
    public:
        explicit Formula(const std::string& expression) try : ast_(ParseFormulaAST(expression)) {}
        catch (...) {
            throw FormulaException("Formula expected");
        }

        [[nodiscard]] Value Evaluate(const SheetInterface& sheet) const override{
            try {
                std::function<double(Position)> params = [&sheet](const Position pos)->double{
                    if (pos.IsValid()) {
                        double zero = 0.0;
                        const auto* cell = sheet.GetCell(pos);
                        if (cell) {
                            const auto& cell_value = cell->GetValue();
                            if (std::holds_alternative<double>(cell_value)) {
                                return std::get<double>(cell_value);
                            } else if (std::holds_alternative<std::string>(cell_value)) {
                                const auto& text = std::get<std::string>(cell_value);
                                if (!text.empty()) {
                                    std::istringstream input(text);
                                    if (input >> zero && input.eof()) {
                                        return zero;
                                    } else {
                                        throw FormulaError(FormulaError::Category::Value);
                                    }
                                } else {
                                    return zero;
                                }
                            } else {
                                throw FormulaError(std::get<FormulaError>(cell_value));
                            }
                        } else {
                            return zero;
                        }
                    } else {
                        throw FormulaError(FormulaError::Category::Ref);
                    }
                };
                return ast_.Execute(params);
            } catch (const FormulaError& er) {
                return er;
            }
        }

        [[nodiscard]] std::string GetExpression() const override {
            std::ostringstream out;
            ast_.PrintFormula(out);
            return out.str();
        }

        [[nodiscard]] std::vector<Position> GetReferencedCells() const override {
            std::vector<Position> cells;
            for (const auto& cell : ast_.GetCells()) {
                if (cell.IsValid()) {
                    cells.push_back(cell);
                } else {
                    continue;
                }
            }
            return cells;
        }
    private:
        FormulaAST ast_;
    };
}// namespace

std::unique_ptr<FormulaInterface> ParseFormula(const std::string& expression) {
    return std::make_unique<Formula>(expression);
}
