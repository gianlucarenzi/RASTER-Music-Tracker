#include "PlatformTypes.h"
#include "ActionTable.h"

#include <map>

namespace {

std::string AcceleratorHint(const std::string& menuText)
{
    size_t tab = menuText.find('\t');
    return tab == std::string::npos ? std::string() : menuText.substr(tab + 1);
}

std::string Escape(std::string cell)
{
    size_t pos = 0;
    while ((pos = cell.find('|', pos)) != std::string::npos) {
        cell.insert(pos, "\\");
        pos += 2;
    }
    return cell;
}

std::string Error(const std::string& message)
{
    return "<br><span style=\"color:red;\">ERROR: " + message + "</span>";
}

struct TRow {
    UINT id = 0;
    bool hasMenu = false;
    const TActionMenuItem* menuItem = nullptr;
    std::string toolBars; // "Main, Block"
    std::string tip;
    std::string status;
    std::string key; // a key without a menu item or a button
};

} // namespace

std::string ActionPlainText(const std::string& menuText)
{
    std::string result;
    for (size_t i = 0; i < menuText.size(); i++) {
        char c = menuText[i];
        if (c == '\t') {
            break;
        }
        if (c == '&') {
            if (i + 1 < menuText.size() && menuText[i + 1] == '&') {
                result += '&';
                i++;
            }
            continue;
        }
        result += c;
    }
    return result;
}

std::string BuildActionTable(const std::vector<TActionMenuItem>& menuItems, const std::vector<TActionToolButton>& buttons, const std::vector<TActionKey>& keys, int& errorCount)
{
    errorCount = 0;
    std::vector<TRow> rows;
    std::map<UINT, size_t> rowOfId;
    auto rowFor = [&](UINT id) -> TRow& {
        auto it = rowOfId.find(id);
        if (it == rowOfId.end()) {
            rowOfId[id] = rows.size();
            rows.emplace_back();
            rows.back().id = id;
            return rows.back();
        }
        return rows[it->second];
    };
    for (const TActionMenuItem& item : menuItems) {
        TRow& row = rowFor(item.id);
        row.hasMenu = true;
        row.menuItem = &item;
    }
    for (const TActionToolButton& button : buttons) {
        TRow& row = rowFor(button.id);
        if (!row.toolBars.empty()) {
            row.toolBars += ", ";
        }
        row.toolBars += button.bar;
        row.tip = button.tip;
        row.status = button.status;
    }

    for (const TActionKey& key : keys) {
        TRow& row = rowFor(key.id);
        row.key = key.key;
    }

    std::string result = "| Access Path | Entry | Accelerator Key | Action |\n|---|---|---|---|\n";
    for (const TRow& row : rows) {
        const TActionMenuItem* item = row.menuItem;

        // The key: the item's real shortcut; a label that displays another key is an error - the label lies.
        // A label's key without a shortcut is a hint the program handles itself (the tracker's own keys).
        std::string realKey = item != nullptr ? item->key : row.key;
        std::string shownKey = item != nullptr ? AcceleratorHint(item->label) : std::string();
        std::string entry = item != nullptr ? ActionPlainText(item->label) : row.tip;
        if (entry.empty()) {
            if (const char* prompt = RmtFindPrompt(row.id)) { // the tooltip part: "status text\ntooltip"
                std::string text = prompt;
                size_t newline = text.find('\n');
                entry = newline != std::string::npos ? text.substr(newline + 1) : text;
            }
        }
        // A button or key without a menu item: the tooltip "Label (Key)" is its label and the key it shows
        if (item == nullptr && entry.size() > 3 && entry.back() == ')') {
            size_t open = entry.rfind(" (");
            if (open != std::string::npos && open > 0) {
                shownKey = entry.substr(open + 2, entry.size() - open - 3);
                entry.resize(open);
            }
        }
        std::string key = realKey.empty() ? shownKey : realKey;
        std::string keyCell = key.empty() ? std::string() : "`" + key + "`";
        if (!realKey.empty() && !shownKey.empty() && realKey != shownKey) {
            keyCell += Error("the menu shows '" + shownKey + "'");
            errorCount++;
        }

        // The prompt: the status text of the string table is the "Action"; a button without a prompt brings its own status text
        std::string action;
        if (const char* prompt = RmtFindPrompt(row.id)) {
            action = prompt;
            size_t newline = action.find('\n'); // status text, newline, tooltip
            if (newline != std::string::npos) {
                action.resize(newline);
            }
        }
        if (action.empty()) {
            action = row.status;
        }

        std::string accessPath;
        if (item != nullptr) {
            accessPath = "Menu ";
            for (size_t i = 0; i < item->path.size(); i++) {
                accessPath += (i > 0 ? " / " : "") + ActionPlainText(item->path[i]);
            }
        }
        if (!row.toolBars.empty()) {
            if (!accessPath.empty()) {
                accessPath += "<br>";
            }
            accessPath += "Tool Bar " + row.toolBars;
        }
        result += "| " + Escape(accessPath) + " | " + Escape(entry) + " | " + keyCell + " | " + Escape(action) + " |\n";
    }
    return result;
}
