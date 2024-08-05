#include <clang-c/Index.h>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <algorithm>

// Define decision-making cursor kinds for cyclomatic complexity
const std::vector<CXCursorKind> decision_kinds = {
    CXCursor_IfStmt, CXCursor_ForStmt, CXCursor_WhileStmt,
    CXCursor_DefaultStmt, CXCursor_CaseStmt,
    CXCursor_ConditionalOperator, CXCursor_BinaryOperator
};

// Get the first child of a cursor
static CXCursor getFirstChild(CXCursor parentCursor)
{
    CXCursor childCursor = clang_getNullCursor();
    clang_visitChildren(
        parentCursor,
        [](CXCursor c, CXCursor parent, CXClientData client_data)
        {
            CXCursor* cursor = static_cast<CXCursor*>(client_data);
            *cursor = c;
            return CXChildVisit_Break;
        },
        &childCursor
    );
    return childCursor;
}

// Extract binary operator from a cursor
std::string getBinaryOperator(CXTranslationUnit translationUnit, CXCursor expressionCursor)
{
    CXToken* expressionTokens;
    unsigned numExpressionTokens;
    clang_tokenize(translationUnit, clang_getCursorExtent(expressionCursor), &expressionTokens, &numExpressionTokens);

    CXCursor leftHandSideCursor = getFirstChild(expressionCursor);
    CXToken* leftHandSideTokens;
    unsigned numLeftHandSideTokens;
    clang_tokenize(translationUnit, clang_getCursorExtent(leftHandSideCursor), &leftHandSideTokens, &numLeftHandSideTokens);

    CXString operatorString = clang_getTokenSpelling(translationUnit, expressionTokens[numLeftHandSideTokens]);
    std::string operatorSymbol(clang_getCString(operatorString));

    clang_disposeString(operatorString);
    clang_disposeTokens(translationUnit, leftHandSideTokens, numLeftHandSideTokens);
    clang_disposeTokens(translationUnit, expressionTokens, numExpressionTokens);

    return operatorSymbol;
}

// Structure to hold edge and node counts
struct EdgeAndNodeCounter
{
    CXTranslationUnit translationUnit;
    int edges = 0;
    int nodes = 0;
};

// Callback function to count edges and nodes
CXChildVisitResult countEdgesAndNodesCallback(CXCursor cursor, CXCursor parent, CXClientData clientData)
{
    EdgeAndNodeCounter* counter = static_cast<EdgeAndNodeCounter*>(clientData);

    const CXCursorKind cursorKind = clang_getCursorKind(cursor);
    if (std::find(decision_kinds.begin(), decision_kinds.end(), cursorKind) != decision_kinds.end())
    {
        if (cursorKind == CXCursor_BinaryOperator)
        {
            std::string operatorSymbol = getBinaryOperator(counter->translationUnit, cursor);
            if (operatorSymbol == "&&" || operatorSymbol == "||")
            {
                counter->edges += 2;
                counter->nodes += 1;
            }
        }
        else
        {
            counter->edges += 2;
            counter->nodes += 1;
        }
    }

    clang_visitChildren(cursor, countEdgesAndNodesCallback, clientData);
    return CXChildVisit_Continue;
}

// Count edges and nodes from a cursor
std::pair<int, int> countEdgesAndNodes(CXCursor cursor, CXTranslationUnit translationUnit)
{
    EdgeAndNodeCounter counter;
    counter.translationUnit = translationUnit;
    countEdgesAndNodesCallback(cursor, clang_getNullCursor(), &counter);
    return {counter.edges, counter.nodes};
}

// Compute cyclomatic complexity
int computeCyclomaticComplexity(CXCursor cursor, CXTranslationUnit translationUnit)
{
    auto counts = countEdgesAndNodes(cursor, translationUnit);
    const int edges = counts.first;
    const int nodes = counts.second + 1;
    return edges - nodes + 2;
}

// Read source code from standard input
std::string readSourceCode()
{
    return std::string((std::istreambuf_iterator<char>(std::cin)), std::istreambuf_iterator<char>());
}

// Create an unsaved file for Clang
CXUnsavedFile createUnsavedFile(const std::string& code)
{
    CXUnsavedFile unsaved_file;
    unsaved_file.Filename = "unsaved.c";
    unsaved_file.Contents = code.c_str();
    unsaved_file.Length = code.length();
    return unsaved_file;
}

// Parse translation unit
CXTranslationUnit parseTranslationUnit(CXIndex index, CXUnsavedFile* unsaved_file)
{
    CXTranslationUnit TU;
    CXErrorCode error = clang_parseTranslationUnit2(index, "unsaved.c", nullptr, 0, unsaved_file, 1, CXTranslationUnit_None, &TU);
    if (error != CXError_Success)
    {
        std::cerr << "Error: Unable to parse translation unit.\n";
        exit(1);
    }
    return TU;
}

// Structure for visiting children
struct ChildVisitor
{
    CXTranslationUnit cxTU;
};

// Callback function for visiting children
CXChildVisitResult visitChildren_callback(CXCursor cursor, CXCursor parent, CXClientData client_data)
{
    ChildVisitor* visitor = static_cast<ChildVisitor*>(client_data);

    if (clang_getCursorKind(cursor) == CXCursor_FunctionDecl)
    {
        CXSourceLocation location = clang_getCursorLocation(cursor);
        unsigned line, column;
        clang_getSpellingLocation(location, NULL, &line, &column, NULL);
        int complexity = computeCyclomaticComplexity(cursor, visitor->cxTU);
        
        CXString functionName = clang_getCursorSpelling(cursor);
        std::string functionNameStr = clang_getCString(functionName);
        clang_disposeString(functionName);

        std::ofstream outputFile("output.cy", std::ios_base::app);
        outputFile << line << " " << functionNameStr << " " << complexity << std::endl;
    }

    return CXChildVisit_Continue;
}

// Visit children of the root cursor
void visitChildren(CXCursor root_cursor, CXTranslationUnit TU)
{
    ChildVisitor visitor;
    visitor.cxTU = TU;
    clang_visitChildren(root_cursor, visitChildren_callback, &visitor);
}

// Main function
int main()
{
    // Truncate the output file at the start
    std::ofstream outputFile("output.cy");
    outputFile.close();

    std::string code = readSourceCode();
    CXUnsavedFile unsaved_file = createUnsavedFile(code);

    CXIndex index = clang_createIndex(0, 0);
    CXTranslationUnit TU = parseTranslationUnit(index, &unsaved_file);

    CXCursor root_cursor = clang_getTranslationUnitCursor(TU);
    visitChildren(root_cursor, TU);

    clang_disposeTranslationUnit(TU);
    clang_disposeIndex(index);
    return 0;
}
