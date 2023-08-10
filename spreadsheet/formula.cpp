#include "formula.h"

#include "FormulaAST.h"

#include <algorithm>
#include <cassert>
#include <cctype>
#include <sstream>

using namespace std::literals;

std::ostream& operator<<(std::ostream& output, FormulaError fe) {
    return output << "#DIV/0!";
}

namespace {
    class Formula : public FormulaInterface {
    public:
// Реализуйте следующие методы:
        explicit Formula(std::string expression) : ast_(ParseFormulaAST(expression)) {}
        Value Evaluate() const override {
            try {
                return ast_.Execute();
            } catch (const FormulaError& evaluate_error) {
                return evaluate_error;
            }
        }
        std::string GetExpression() const override {
            std::ostringstream expr;
            ast_.PrintFormula(expr);
            return expr.str();
        }

    private:
        FormulaAST ast_;
    };
}  // namespace

std::unique_ptr<FormulaInterface> ParseFormula(std::string expression) {
    return std::make_unique<Formula>(std::move(expression));
}
