#include "cell.h"

#include <cassert>
#include <iostream>
#include <string>
#include <optional>

Cell::Cell() : impl_(std::make_unique<EmptyImpl>()) {}
Cell::~Cell() = default;

void Cell::Set(std::string text) {
    if (text.empty()) {
        impl_ = std::make_unique<EmptyImpl>();
        return;
    } else if (text.size() >= 2 && text.at(0) == FORMULA_SIGN) {
        impl_ = std::move(std::make_unique<FormulaImpl>(std::move(text)));
        return;
    } else {
        impl_ = std::move(std::make_unique<TextImpl>(std::move(text)));
        return;
    }
}

void Cell::Clear() {
    impl_ = std::make_unique<EmptyImpl>();
}

Cell::Value Cell::GetValue() const {
    return impl_->GetValue();
}
std::string Cell::GetText() const {
    return impl_->GetText();
}

Cell::Value Cell::EmptyImpl::GetValue() const {
    return "";
}
std::string Cell::EmptyImpl::GetText() const {
    return "";
}

Cell::TextImpl::TextImpl(std::string text)
        : text_(std::move(text)) {}

Cell::Value Cell::TextImpl::GetValue() const {
    if (text_.empty()) {
        throw FormulaError("Empty text");
    } else if (text_.at(0) == ESCAPE_SIGN) {
        return text_.substr(1);
    } else {
        return text_;
    }
}

std::string Cell::TextImpl::GetText() const {
    return text_;
}

Cell::FormulaImpl::FormulaImpl(std::string text)
        : formula_(ParseFormula(text.substr(1))) {}

Cell::Value Cell::FormulaImpl::GetValue() const {
    auto value = formula_->Evaluate();
    if (std::holds_alternative<double>(value)) {
        return std::get<double>(value);
    } else {
        return std::get<FormulaError>(value);
    }
}

std::string Cell::FormulaImpl::GetText() const {
    return FORMULA_SIGN + formula_->GetExpression();
}
