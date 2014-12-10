#ifndef TOKEN_H
#define TOKEN_H

#include "common/utils.h"
#include <QString>
#include <QList>
#include <QSharedPointer>

/** @file */

/**
 * @def TOKEN_TYPE_MASK_DB_OBJECT
 *
 * Bit mask used to test Token::Type for representing any database object name, in any form.
 * It's used in Token::isDbObjectType().
 */
#define TOKEN_TYPE_MASK_DB_OBJECT 0x1000

struct Token;

/**
 * @brief Shared pointer to the Token.
 */
typedef QSharedPointer<Token> TokenPtr;

/**
 * @brief SQL query entity representing isolated part of the query.
 *
 * Tokens are generated by Lexer. Each token represents isolated part of the query,
 * like a name, a number, an operator, a string, a keyword, or a comment, etc.
 *
 * In other words, tokenizing SQL query is splitting it into logical parts.
 *
 * Each token has a type, a value and position indexes of where it starts and where it ends (in the query string).
 *
 * Tokens are used mainly by the Parser to perform syntax analysis, but they can be also very helpful
 * in other areas. They provide easy way for safe manipulation on the query string, without worrying
 * about counting open or close characters of the string, etc. If the string has a single-quote used twice inside,
 * this is still a regular SQL string and in that case there will be only a single string token representing it.
 *
 * If you're constructing Token outside of the Lemon parser, you should be interested only in 4 constructors:
 * Token(), Token(QString value), Token(Type type, QString value)
 * and Token(Type type, QString value, qint64 start, qint64 end).
 * Rest of the constructors are to be used from the Lemon parser, as they require Lemon token ID to be provided.
 *
 * You will usually have the most to do with tokens when dealing with SqliteStatement and its 2 members:
 * SqliteStatement::tokens and SqliteStatement::tokenMap.
 */
struct API_EXPORT Token
{
    /**
     * @brief Token type.
     *
     * There are 2 kind of types - regular and context-oriented.
     *
     * Regular types are those defined by the SQL grammar and they represent real tokens. A special token type
     * from group of regular types is the Type::INVALID, which means, that the character(s) encountered
     * by the Lexer are invalid in SQL syntax understanding, or when there was no more query characters to read
     * (which in this case means that tokenizing ended before this token).
     *
     * The context-oriented types are special meta types used by the Parser to probe potential candidates
     * for next valid token when Parser::getNextTokenCandidates() is called. They are then processed by
     * CompletionHelper. You won't deal with this kind of token types on regular basis. Context-oriented
     * types are those starting with <tt>CTX_</tt>.
     */
    enum Type
    {
        INVALID = 0x0001,               /**< Invalid token, or no more tokens available from Lexer. */
        OTHER = 0x1002,                 /**< A name, a word. */
        STRING = 0x0003,                /**< A string (value will be stripped of the surrounding quotes). */
        COMMENT = 0x0004,               /**< A comment, including starting/ending markers. */
        FLOAT = 0x0005,                 /**< A decimal number. */
        INTEGER = 0x0006,               /**< An integer number. */
        BIND_PARAM = 0x0007,            /**< A bind parameter (<tt>:param</tt>, <tt>\@param</tt>, or <tt>?</tt>). */
        OPERATOR = 0x0008,              /**< An operator (like <tt>";"</tt>, <tt>"+"</tt>, <tt>","</tt>, etc). */
        PAR_LEFT = 0x0009,              /**< A left parenthesis (<tt>"("</tt>). */
        PAR_RIGHT = 0x0010,             /**< A right parenthesis (<tt>")"</tt>). */
        SPACE = 0x0011,                 /**< White space(s), including new line characters and tabs. */
        BLOB = 0x0012,                  /**< Literal BLOB value (<tt>X'...'</tt> or <tt>x'...'</tt>). */
        KEYWORD = 0x0013,               /**< A keyword (see getKeywords3() and getKeywords2()). */
        CTX_COLUMN = 0x1014,            /**< Existing column name is valid at this token position. */
        CTX_TABLE = 0x1015,             /**< Existing table name is valid at this token potision. */
        CTX_DATABASE = 0x1016,          /**< Database name is valid at this token position. */
        CTX_FUNCTION = 0x0017,          /**< Function name is valid at this token position. */
        CTX_COLLATION = 0x0018,         /**< Collation name is valid at this token position. */
        CTX_INDEX = 0x1019,             /**< Existing index name is valid at this token position. */
        CTX_TRIGGER = 0x1020,           /**< Existing trigger name is valid at this token position. */
        CTX_VIEW = 0x1021,              /**< View name is valid at this token position. */
        CTX_JOIN_OPTS = 0x0022,         /**< JOIN keywords are valid at this token position (see getJoinKeywords()). */
        CTX_TABLE_NEW = 0x0023,         /**< Name for new table is valid at this token position. */
        CTX_INDEX_NEW = 0x0024,         /**< Name for new index is valid at this token position. */
        CTX_VIEW_NEW = 0x0025,          /**< Name for new view is valid at this token position. */
        CTX_TRIGGER_NEW = 0x0026,       /**< Name for new trigger is valid at this token position. */
        CTX_ALIAS = 0x0027,             /**< Alias name is valid at this token position. */
        CTX_TRANSACTION = 0x0028,       /**< Transaction name is valid at this token position. */
        CTX_COLUMN_NEW = 0x0029,        /**< Name for the new column is valid at this token position. */
        CTX_COLUMN_TYPE = 0x0030,       /**< Data type for the new column is valid at this token position. */
        CTX_CONSTRAINT = 0x0031,        /**< Name for the new constraint is valid at this token position. */
        CTX_FK_MATCH = 0x0032,          /**< MATCH keywords are valid at this token position (see getFkMatchKeywords()). */
        CTX_PRAGMA = 0x0033,            /**< Pragma name is valid at this token position. */
        CTX_ROWID_KW = 0x0034,          /**< ROWID keywords is valid at this token position (see isRowIdKeyword()). */
        CTX_NEW_KW = 0x0035,            /**< The <tt>NEW</tt> keyword is valid at this token position. */
        CTX_OLD_KW = 0x0036,            /**< The <tt>OLD</tt> keyword is valid at this token position. */
        CTX_ERROR_MESSAGE = 0x0037      /**< Error message string is valid at this token position. */
    };

    /**
     * @brief Creates empty token with type Type::INVALID.
     *
     * Lemon token ID is set to 0 and start/end positions are set to -1.
     */
    Token();

    /**
     * @brief Creates fully defined token.
     * @param lemonType Lemon token ID to use (see sqlite2_parser.h and sqlite3_parser.h).
     * @param type Token type.
     * @param value Value of the token.
     * @param start Start position of the token (index of the first character in the query).
     * @param end End position of the token (index of last character in the query).
     *
     * This constructor is intended to be used from Lemon parser only. For other use cases
     * use constructors without the \p lemonType parameter, unless you need it and you know what you're doing.
     */
    Token(int lemonType, Type type, QString value, qint64 start, qint64 end);

    /**
     * @overload
     */
    Token(int lemonType, Type type, QChar value, qint64 start, qint64 end);

    /**
     * @overload
     */
    Token(int lemonType, Type type, QString value);

    /**
     * @brief Creates token with type Type::INVALID and given value.
     * @param value Value for the token.
     *
     * Start/end positions are set to -1.
     */
    explicit Token(QString value);

    /**
     * @brief Creates token of given type and with given value.
     * @param type Type for the token.
     * @param value Value for the token.
     *
     * Start/end positions are set to -1.
     */
    Token(Type type, QString value);

    /**
     * @brief Creates fully defined token.
     * @param type Type of the token.
     * @param value Value for the token.
     * @param start Start position of the token (index of the first character in the query).
     * @param end End position of the token (index of last character in the query).
     */
    Token(Type type, QString value, qint64 start, qint64 end);

    /**
     * @brief Destructor declared as virtual. Does nothing in this implementation.
     */
    virtual ~Token();

    /**
     * @brief Serializes token to human readable form.
     * @return Token values in format: <tt>{type value start end}</tt>
     */
    QString toString();

    /**
     * @brief Converts given token type into its string representation.
     * @param type Type to convert.
     * @return Type as a string (same as textual representation of the enum in the code).
     */
    static const QString typeToString(Type type);

    /**
     * @brief Provides range of the token in the query.
     * @return Start and end character index in relation to the query it comes from.
     */
    Range getRange();

    /**
     * @brief Tests whether this token represents any kind of whitespace.
     * @return true if it's a whitespace, or false otherwise.
     *
     * Note, that from SQL perspective also comments are whitespaces.
     */
    bool isWhitespace() const;

    /**
     * @brief Tests whether this token represents separating value (like an operator, or parenthesis) in SQL understanding.
     * @return true if it's a separating token, or false otherwise.
     */
    bool isSeparating() const;

    /**
     * @brief Tests whether this token is representing any kind of database object name.
     * @return true if the token is the name an object, or false otherwise.
     *
     * From regular token types only the Type::OTHER represents.
     * For context-oriented types there are several types representing object name.
     * Use this method to find out which is and which is not.
     *
     * You won't need to use this method in most cases. It's useful only to CompletionHelper
     * for now.
     */
    bool isDbObjectType() const;

    /**
     * @brief Converts token's type into a string representation.
     * @return Token type as a string.
     */
    QString typeString() const;

    /**
     * @brief Compares other token to this token.
     * @param other Other token to compare.
     * @return 1 if tokens are equal, 0 if they're different.
     *
     * Tokens are equal then 4 members are equal: type, value, start and end.
     * The lemonType member is ignored by this operator.
     */
    int operator==(const Token& other);

    /**
     * @brief Compares other token to this token and tells which one is greater.
     * @param other Other token to compare.
     * @return true if the other token is greater than this one, or false if it's smaller or equal.
     *
     * This operator compares only 2 members: the start and the end indexes. This operator
     * is used to sort tokens by the character position they occurred at.
     *
     * The start value has higher precedence than the end value, but if start values are equal,
     * then the end value is conclusive.
     */
    bool operator<(const Token& other) const;

    /**
     * @brief Lemon token ID. Used by the Parser class only.
     */
    int lemonType;

    /**
     * @brief Token type, describing general class of the token.
     */
    Type type;

    /**
     * @brief Literal value of the token, captured directly from the query.
     */
    QString value = QString::null;

    /**
     * @brief Start position (first character index) of the token in the query.
     */
    qint64 start;

    /**
     * @brief End position (last character index) of the token in the query.
     */
    qint64 end;
};

/**
 * @brief qHash implementation for TokenPtr, so it can be used as a key in QHash and QSet.
 * @param token Token that the hash code is calculated for.
 * @return Unique integer value for the token.
 */
uint qHash(const TokenPtr& token);

struct TolerantToken;
/**
 * @brief Shared pointer to TolerantToken.
 */
typedef QSharedPointer<TolerantToken> TolerantTokenPtr;

/**
 * @brief Variation of token that has additional "invalid" flag.
 *
 * TolerantToken is used by Lexer to tolerate unfinished comments, like when you start the
 * comment at the end of the query, but you never close the comment. This is tolerable case,
 * while not entire correct by the syntax.
 *
 * In such cases the syntax highlighter must be aware of the token being invalid, so the proper
 * state is marked for the paragraph.
 */
struct TolerantToken : public Token
{
    /**
     * @brief Invalid state flag for the token.
     */
    bool invalid = false;
};

/**
 * @brief Ordered list of tokens.
 *
 * This is pretty much a QList of Token pointers, but it also provides some
 * utility methods regarding this collection, which is very common in SQLiteStudio.
 */
class API_EXPORT TokenList : public QList<TokenPtr>
{
    public:
        /**
         * @brief Creates empty list.
         */
        TokenList();

        /**
         * @brief Creates list filled with the same entries as the other list.
         * @param other List to copy pointers from.
         */
        TokenList(const QList<TokenPtr>& other);

        /**
         * @brief Serializes contents of the list into readable form.
         * @return Contents in format: <tt>{type1 value1 start1 end1} {type2 value2 start2 end2} ...</tt>.
         *
         * Tokens are serialized with Token::toString(), then all serialized values are joined with single whitespace
         * into the QString.
         */
        QString toString() const;

        /**
         * @brief Serializes tokens from the list into strings.
         * @return List of tokens serialized into strings.
         *
         * Tokens are serialized with Token::toString().
         */
        QStringList toStringList() const;

        /**
         * @brief Provides index of first occurrence of the token in the list.
         * @param token Token to look for.
         * @return Index of the token, or -1 if token was not found.
         */
        int indexOf(TokenPtr token) const;

        /**
         * @brief Provides index of first occurrence of the token with given type.
         * @param type Toke type to look for.
         * @return Index of the token, or -1 if token was not found.
         */
        int indexOf(Token::Type type) const;

        /**
         * @brief Provides index of first occurrence of the token with given type and value.
         * @param type Token type to look for.
         * @param value Token value to look for.
         * @param caseSensitivity Should value lookup be case sensitive?
         * @return Index of the token, or -1 if token was not found.
         */
        int indexOf(Token::Type type, const QString& value, Qt::CaseSensitivity caseSensitivity = Qt::CaseSensitive) const;

        /**
         * @brief Provides index of first occurrence of the token with given value.
         * @param value Value to look for.
         * @param caseSensitivity Should value lookup be case sensitive?
         * @return Index of the token, or -1 if token was not found.
         */
        int indexOf(const QString& value, Qt::CaseSensitivity caseSensitivity = Qt::CaseSensitive) const;

        /**
         * @brief Provides index of last occurrence of the token in the list.
         * @param token Token to look for.
         * @return Index of the token, or -1 if token was not found.
         */
        int lastIndexOf(TokenPtr token) const;

        /**
         * @brief Provides index of last occurrence of the token with given type.
         * @param type Token type to look for.
         * @return Index of the token, or -1 if token was not found.
         */
        int lastIndexOf(Token::Type type) const;

        /**
         * @brief Provides index of last occurrence of the token with given type and value.
         * @param type Token type to look for.
         * @param value Token value to look for.
         * @param caseSensitivity Should value lookup be case sensitive?
         * @return Index of the token, or -1 if token was not found.
         */
        int lastIndexOf(Token::Type type, const QString& value, Qt::CaseSensitivity caseSensitivity = Qt::CaseSensitive) const;

        /**
         * @brief Provides index of last occurrence of the token with given value.
         * @param value Value to look for.
         * @param caseSensitivity Should value lookup be case sensitive?
         * @return Index of the token, or -1 if token was not found.
         */
        int lastIndexOf(const QString& value, Qt::CaseSensitivity caseSensitivity = Qt::CaseSensitive) const;

        /**
         * @brief Finds first token with given type in the list.
         * @param type Type to look for.
         * @return Token found, or null pointer if it was not found.
         */
        TokenPtr find(Token::Type type) const;

        /**
         * @brief Finds first token with given type and value.
         * @param type Type to look for.
         * @param value Token value to look for.
         * @param caseSensitivity Should value lookup be case sensitive?
         * @return Token found, or null pointer if it was not found.
         */
        TokenPtr find(Token::Type type, const QString& value, Qt::CaseSensitivity caseSensitivity = Qt::CaseSensitive) const;

        /**
         * @brief Finds first token with given type and value.
         * @param value Token value to look for.
         * @param caseSensitivity Should value lookup be case sensitive?
         * @return Token found, or null pointer if it was not found.
         */
        TokenPtr find(const QString& value, Qt::CaseSensitivity caseSensitivity = Qt::CaseSensitive) const;

        /**
         * @brief Finds last token with given type in the list.
         * @param type Type to look for.
         * @return Token found, or null pointer if it was not found.
         */
        TokenPtr findLast(Token::Type type) const;

        /**
         * @brief Finds last token with given type and value.
         * @param type Type to look for.
         * @param value Token value to look for.
         * @param caseSensitivity Should value lookup be case sensitive?
         * @return Token found, or null pointer if it was not found.
         */
        TokenPtr findLast(Token::Type type, const QString& value, Qt::CaseSensitivity caseSensitivity = Qt::CaseSensitive) const;

        /**
         * @brief Finds last token with given value.
         * @param value Token value to look for.
         * @param caseSensitivity Should value lookup be case sensitive?
         * @return Token found, or null pointer if it was not found.
         */
        TokenPtr findLast(const QString& value, Qt::CaseSensitivity caseSensitivity = Qt::CaseSensitive) const;

        /**
         * @brief Finds token that according to its start and end values covers given character position.
         * @param cursorPosition Position to get token for.
         * @return Token found, or null pointer if it was not found.
         */
        TokenPtr atCursorPosition(quint64 cursorPosition) const;

        /**
         * @brief Inserts tokens at given position in this list.
         * @param i Position to insert at.
         * @param list List of tokens to insert.
         */
        void insert(int i, const TokenList& list);

        /**
         * @brief Inserts a token at given position in this list.
         * @param i Position to insert at.
         * @param token Token to insert.
         */
        void insert(int i, TokenPtr token);

        /**
         * @brief Puts all tokens from the other list to this list.
         * @param other List to get tokens from.
         * @return Reference to this list.
         *
         * It erases any previous entries in this list, just like you would normally expect for the <tt>=</tt> operator.
         */
        TokenList& operator=(const QList<TokenPtr>& other);

        /**
         * @brief Detokenizes all tokens from this list.
         * @return String from all tokens.
         *
         * This is just a convenient method to call Lexer::detokenize() on this list.
         */
        QString detokenize() const;

        /**
         * @brief Replaces tokens on this list with other tokens.
         * @param startIdx Position of the first token in this list to replace.
         * @param length Number of tokens to replace.
         * @param newTokens New tokens to put in place of removed ones.
         */
        void replace(int startIdx, int length, const TokenList& newTokens);

        /**
         * @brief Replaces tokens on this list with another token.
         * @param startIdx Position of the first token in this list to replace.
         * @param length Number of tokens to replace.
         * @param newToken New token to put in place of removed ones.
         */
        void replace(int startIdx, int length, TokenPtr newToken);

        /**
         * @brief Replaces token with another token.
         * @param startIdx Position of the token in this list to replace.
         * @param newToken New token to put in place of removed one.
         */
        void replace(int startIdx, TokenPtr newToken);

        /**
         * @brief Replaces token on this list with other tokens.
         * @param startIdx Position of the token in this list to replace.
         * @param newTokens New tokens to put in place of removed ones.
         */
        void replace(int startIdx, const TokenList& newTokens);

        /**
         * @brief Replaces tokens on this list with other tokens.
         * @param startToken First token to replace.
         * @param endToken Last token to replace.
         * @param newTokens New tokens to put in place of removed ones.
         * @return Number of tokens replaced.
         *
         * If either \p startToken or \p endToken were not found on the list, this method does nothing
         * and returns 0.
         */
        int replace(TokenPtr startToken, TokenPtr endToken, const TokenList& newTokens);

        /**
         * @brief Replaces tokens on this list with other tokens.
         * @param startToken First token to replace.
         * @param endToken Last token to replace.
         * @param newToken New token to put in place of removed ones.
         * @return Number of tokens replaced.
         *
         * If either \p startToken or \p endToken were not found on the list, this method does nothing
         * and returns 0.
         */
        int replace(TokenPtr startToken, TokenPtr endToken, TokenPtr newToken);

        /**
         * @brief Replaces token on this list with another token.
         * @param oldToken Token to replace.
         * @param newToken Token to replace with.
         * @return true if \p oldToken was found and replaced, or false otherwise.
         */
        bool replace(TokenPtr oldToken, TokenPtr newToken);

        /**
         * @brief Replaces token on this list with other tokens.
         * @param oldToken Token to replace.
         * @param newTokens Tokens to replace with.
         * @return true if \p oldToken was found and replaced, or false otherwise.
         */
        bool replace(TokenPtr oldToken, const TokenList& newTokens);

        /**
         * @brief Removes tokens from the list.
         * @param startToken First token to remove.
         * @param endToken Last token to remove.
         * @return true if \p startToken and \p endToken were found in the list and removed, or false otherwise.
         *
         * In case \p startToken is placed after \p endToken, this method does nothing and returns false.
         */
        bool remove(TokenPtr startToken, TokenPtr endToken);

        /**
         * @brief Removes first token of given type from the list.
         * @param type Token type to remove.
         * @return true if token was located and removed, or false otherwise.
         */
        bool remove(Token::Type type);

        /**
         * @brief Removes all white-space tokens from the beginning of the list.
         *
         * White-space tokens are tested with Token::isWhitespace().
         */
        TokenList &trimLeft();

        /**
         * @brief Removes all white-space tokens from the end of the list.
         *
         * White-space tokens are tested with Token::isWhitespace().
         */
        TokenList &trimRight();

        /**
         * @brief Removes all white-space tokens from both the beginning and the end of the list.
         *
         * White-space tokens are tested with Token::isWhitespace().
         */
        TokenList &trim();

        /**
         * @brief Removes all tokens that match given criteria from the beginning of the list.
         * @param type Token type to remove.
         * @param alsoTrim Token value to remove.
         *
         * This method is an extension to the regular trimLeft(). It removes white-space tokens,
         * as well as tokens that are of given \p type and have given \p value (both conditions must be met).
         */
        TokenList &trimLeft(Token::Type type, const QString& alsoTrim);

        /**
         * @brief Removes all tokens that match given criteria from the end of the list.
         * @param type Token type to remove.
         * @param alsoTrim Token value to remove.
         *
         * This method is an extension to the regular trimRight(). It removes white-space tokens,
         * as well as tokens that are of given \p type and have given \p value (both conditions must be met).
         */
        TokenList &trimRight(Token::Type type, const QString& alsoTrim);

        /**
         * @brief Removes all tokens that match given criteria from the beginning and the end of the list.
         * @param type Token type to remove.
         * @param alsoTrim Token value to remove.
         *
         * This method is an extension to the regular trim(). It removes white-space tokens,
         * as well as tokens that are of given \p type and have given \p value (both conditions must be met).
         */
        TokenList &trim(Token::Type type, const QString& alsoTrim);

        /**
         * @brief Creates list of tokens from this list, letting through only tokens of given type.
         * @param type Type of tokens to provide in the new list.
         * @return List of tokens from this list matching given \p type.
         */
        TokenList filter(Token::Type type) const;

        /**
         * @brief Creates list of tokens from this list, letting through only tokens that are not a whitespace.
         * @return List of tokens from this list that are not a whitespace.
         *
         * The condition to test if tokens is a whitespace is a call to Token::isWhitespace().
         */
        TokenList filterWhiteSpaces() const;

        /**
         * @brief Returns sub-list of tokens from this list.
         * @param pos Position to start sublist from.
         * @param length Number of tokens to get from this list. If -1 (default), then all from the \p pos to the end.
         * @return Sub-list of tokens from this list.
         */
        TokenList mid(int pos, int length = -1) const;

    private:
        /**
         * @brief Finds first occurrence of token with given type.
         * @param type Type of token to look for.
         * @param idx Pointer to integer variable to store position in.
         * @return Token found, or null pointer if token was not found.
         *
         * If \p idx is not null, then the position of token found is stored in it. If token was not found,
         * then -1 is stored in the \p idx.
         *
         * This method is used by the public findFirst() and indexOf() methods, as they share common logic.
         */
        TokenPtr findFirst(Token::Type type, int* idx) const;

        /**
         * @brief Finds first occurrence of token with given type and value.
         * @param type Type of token to look for.
         * @param value Value of the token to look for.
         * @param caseSensitivity Should value lookup be case sensitive?
         * @param idx Pointer to integer variable to store position in.
         * @return Token found, or null pointer if token was not found.
         *
         * If \p idx is not null, then the position of token found is stored in it. If token was not found,
         * then -1 is stored in the \p idx.
         *
         * This method is used by the public findFirst() and indexOf() methods, as they share common logic.
         */
        TokenPtr findFirst(Token::Type type, const QString& value, Qt::CaseSensitivity caseSensitivity, int* idx) const;

        /**
         * @brief Finds first occurrence of token with given value.
         * @param value Value of the token to look for.
         * @param caseSensitivity Should value lookup be case sensitive?
         * @param idx Pointer to integer variable to store position in.
         * @return Token found, or null pointer if token was not found.
         *
         * If \p idx is not null, then the position of token found is stored in it. If token was not found,
         * then -1 is stored in the \p idx.
         *
         * This method is used by the public findFirst() and indexOf() methods, as they share common logic.
         */
        TokenPtr findFirst(const QString& value, Qt::CaseSensitivity caseSensitivity, int* idx) const;

        /**
         * @brief Finds last occurrence of token with given type.
         * @param type Type of token to look for.
         * @param idx Pointer to integer variable to store position in.
         * @return Token found, or null pointer if token was not found.
         *
         * If \p idx is not null, then the position of token found is stored in it. If token was not found,
         * then -1 is stored in the \p idx.
         *
         * This method is used by the public findLast() and lastIndexOf() methods, as they share common logic.
         */
        TokenPtr findLast(Token::Type type, int* idx) const;

        /**
         * @brief Finds last occurrence of token with given type and value.
         * @param type Type of token to look for.
         * @param value Value of the token to look for.
         * @param caseSensitivity Should value lookup be case sensitive?
         * @param idx Pointer to integer variable to store position in.
         * @return Token found, or null pointer if token was not found.
         *
         * If \p idx is not null, then the position of token found is stored in it. If token was not found,
         * then -1 is stored in the \p idx.
         *
         * This method is used by the public findLast() and lastIndexOf() methods, as they share common logic.
         */
        TokenPtr findLast(Token::Type type, const QString& value, Qt::CaseSensitivity caseSensitivity, int* idx) const;

        /**
         * @brief Finds last occurrence of token with given value.
         * @param value Value of the token to look for.
         * @param caseSensitivity Should value lookup be case sensitive?
         * @param idx Pointer to integer variable to store position in.
         * @return Token found, or null pointer if token was not found.
         *
         * If \p idx is not null, then the position of token found is stored in it. If token was not found,
         * then -1 is stored in the \p idx.
         *
         * This method is used by the public findLast() and lastIndexOf() methods, as they share common logic.
         */
        TokenPtr findLast(const QString& value, Qt::CaseSensitivity caseSensitivity, int* idx) const;
};


#endif // TOKEN_H
