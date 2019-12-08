#pragma once
#include "zep/syntax.hpp"
#include <list>
#include <string>
#include <unordered_map>

namespace Zep
{

class ZepSyntaxAdorn_RainbowBrackets : public ZepSyntaxAdorn
{
public:
    using TParent = ZepSyntaxAdorn;
    ZepSyntaxAdorn_RainbowBrackets(ZepSyntax& syntax, ZepBuffer& buffer);
    ~ZepSyntaxAdorn_RainbowBrackets() override;

    void Notify(std::shared_ptr<ZepMessage> message) override;
    auto GetSyntaxAt(int32_t offset, bool& found) const -> SyntaxData override;

    virtual void Clear(int32_t start, int32_t end);
    virtual void Insert(int32_t start, int32_t end);
    virtual void Update(int32_t start, int32_t end);

private:
    void RefreshBrackets();
    enum class BracketType
    {
        Bracket = 0,
        Brace = 1,
        Group = 2,
        Max = 3
    };

    struct Bracket
    {
        int32_t indent{};
        BracketType type{};
        bool is_open{};
        bool valid = true;
    };
    std::map<BufferLocation, Bracket> m_brackets;
};

} // namespace Zep
