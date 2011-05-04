-- 
-- Copyright (c) Tuomo Valkonen 2006.
--
-- This program is free software; you can redistribute it and/or modify
-- it under the terms of the GNU General Public License as published by
-- the Free Software Foundation; either version 2 of the License, or
-- (at your option) any later version.
--

module Kuu.Token (
    Token(..),
    TokenStream,
    ParseError,
    tokenize,
    filter_comments,
    number,
    integer
) where
    
import Text.ParserCombinators.Parsec
import Text.ParserCombinators.Parsec.Pos ( SourcePos )
import Data.Char ( ord )
import Data.List ( sortBy )
import Control.Monad ( liftM ) 

data Token = TokString String
           | TokNumber Double
           | TokComment String
           | TokIdentifier String
           | TokReserved String
           | TokSpecial String
           | TokOperator String
           deriving Show

type TokenStream = [(SourcePos, Token)]

-----------

filter_comments :: TokenStream -> TokenStream
filter_comments = filter not_comment
    where 
        not_comment (_, TokComment _) = False
        not_comment _ = True

tokenize :: FilePath -> String -> Either ParseError TokenStream
tokenize = parse pars

pars = do
    optional (try (string "#!" >> skipMany (noneOf "\n")))
    skipMany space
    choice[
        eof >> return [],
        do
            p <- getPosition
            t <- tok
            liftM ((p, t):) $ pars
        ]

tok = identifier_or_operator
    <|> string_lit
    <|> comment
    <|> number
    <|> operator_or_special

-----------

sp0 = ["...", ".", ":", ",", ";", "=", "(", ")", "[", "]", "{", "}"]
op0 = ["+", "-", "*", "/", "%", "^", "..", "<=", "<", ">=", ">", "==", "~=", "#"]
op1 = ["and", "or", "not"]

reserved = ["end", "in", "repeat", "break", "false", "local", "return",
            "do", "for", "nil", "then", "else", "function", "true",
            "elseif", "if", "until", "while"]

-----------

ihead = "_" ++ ['a'..'z'] ++ ['A'..'Z']
itail = ihead ++ ['0'..'9']

identifier_or_operator = do
    c <- oneOf ihead
    s <- liftM (c:) $ many $ oneOf itail
    case (s `elem` op1, s `elem` reserved) of
        (True, _) -> return $ TokOperator s
        (_, True) -> return $ TokReserved s
        _         -> return $ TokIdentifier s

-----------

string_lit = q_string '\''
    <|> q_string '"'
    <|> ml_string

skip chs =
    (oneOf chs >> return ()) <|> return ()
    
ml_string = do
    try $ string "[["
    skip "\n"
    s <- dcs
    return $ TokString s
    where
        dcs   = do try $ string "]]"
	           return []
	    <|> do try $ string "[["
	           s <- dcs
		   s2 <- dcs
		   return $ concat ["[[", s, "]]", s2]
            <|> do c <- anyChar
	           s <- dcs
		   return (c:s)
            <?> "unexpected end of string"
    
q_string q = do
    char q
    s <- dcs
    return $ TokString s
    where
        dcs =   do { char q; return [] }
	    <|> do { c <- escaped <|> noneOf "\n"; s <- dcs; return (c:s) }
	escaped = do
	    char '\\'
            (integer >>= return . toEnum) <|> (anyChar >>= unescape)

unescape 'a' = return '\a'
unescape 'b' = return '\b'
unescape 'f' = return '\f'
unescape 'n' = return '\n'
unescape '\n'= return '\n'
unescape 'r' = return '\r'
unescape 't' = return '\t'
unescape 'v' = return '\v'
unescape '\\'= return '\\'
unescape '"' = return '"'
unescape '\'' = return '\''
unescape '[' = return '['
unescape ']' = return ']'
unescape _   = fail "Invalid escaped character"
    

-----------

comment = do
    try $ string "--"
    c <-    do { (TokString s) <- ml_string; return s }
        <|> do { many (noneOf "\n") }
    return $ TokComment c

-----------

number = do
    i <- liftM fromIntegral $ integer
    d <- (char '.' >> decimal_part) <|> return (0::Double)
    e <- (oneOf "eE" >> exponent_part) <|> return 0
    return $ TokNumber $ (i+d)*10^^e

pp10 = map (10^) [0..]
np10 = map (10^^) [-1,-2..]

integer = do
    d <- many1 $ ldigit
    return $ sum $ zipWith (*) (reverse d) pp10

decimal_part :: Parser Double
decimal_part = do
    d <- many1 $ (liftM fromIntegral $ ldigit)
    return $ sum $ zipWith (*) d np10

exponent_part = do
    sign <- do { char '+'; return 1 }
        <|> do { char '-'; return (-1) }
        <|> do { return 1 }
    i <- integer
    return $ sign*i

ldigit = do
    d <- oneOf ['0'..'9']
    return $ (ord d) - (ord '0')

-----------

operator_or_special = do
    try $ choice os
    where
        fso c s = (length s, (try $ string s) >> return (c s))
        o = map (fso TokOperator) op0
        s = map (fso TokSpecial) sp0
        sf (l, _) (v, _) = compare v l
        os = map snd $ sortBy sf (o++s)

