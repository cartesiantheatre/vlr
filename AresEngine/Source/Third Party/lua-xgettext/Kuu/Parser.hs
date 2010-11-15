-- 
-- Copyright (c) Tuomo Valkonen 2006.
--
-- This program is free software; you can redistribute it and/or modify
-- it under the terms of the GNU General Public License as published by
-- the Free Software Foundation; either version 2 of the License, or
-- (at your option) any later version.
--

module Kuu.Parser(
    parse_chunk,
    ParseError
) where

import Text.ParserCombinators.Parsec
import Text.ParserCombinators.Parsec.Expr

import Data.List
import Control.Monad ( when, liftM )
import Debug.Trace

import Kuu.Token ( Token(..), TokenStream )
import Kuu.AST

-----------

mytoken :: (Token -> Maybe a) -> GenParser (SourcePos, Token) () a
mytoken test = 
    token showtok postok testtok
    where
        showtok (pos, tok) = show tok
        postok  (pos, tok) = pos
        testtok (pos, tok) = test tok

tok_string = mytoken (\f -> case f of
                                (TokString a) -> Just a
                                _ -> Nothing)

tok_number = mytoken (\f -> case f of
                                (TokNumber a) -> Just a
                                _ -> Nothing)

tok_identifier = mytoken (\f -> case f of
                                (TokIdentifier a) -> Just a
                                _ -> Nothing)

tok_reserved = mytoken (\f -> case f of
                                (TokReserved a) -> Just a
                                _ -> Nothing)

tok_special = mytoken (\f -> case f of
                                (TokSpecial a) -> Just a
                                _ -> Nothing)

tok_operator = mytoken (\f -> case f of
                                (TokOperator a) -> Just a
                                _ -> Nothing)

reserved s = mytoken (\f -> case f of
                                (TokReserved t) | s == t -> Just ()
                                _ -> Nothing)

special s = mytoken (\f -> case f of
                                (TokSpecial t) | s == t -> Just ()
                                _ -> Nothing)

operator s = mytoken (\f -> case f of
                                (TokOperator t) | s == t -> Just ()
                                _ -> Nothing)

parens p   = between (special "(") (special ")") p
braces p   = between (special "{") (special "}") p
brackets p = between (special "[") (special "]") p

-----------

parse_chunk :: FilePath -> TokenStream -> Either ParseError Block
parse_chunk = parse $ do
    r <- block
    eof
    return r

-----------

block = liftM Block $ many $ do
    s <- stat
    option () (special ";")
    return s

stat = choice [
    stmt_do,
    stmt_while,
    stmt_repeat,
    stmt_if,
    stmt_return,
    stmt_break,
    stmt_for,
    stmt_function,
    stmt_local,
    stmt_assignment_or_funccall
    ]


stmt_do = do
    reserved "do"
    b <- block
    reserved "end"
    return $ SDo b

stmt_while = do
    reserved "while"
    e <- exp_exp
    reserved "do"
    b <- block
    reserved "end"
    return $ SWhile e b

stmt_repeat = do
    reserved "repeat"
    b <- block
    reserved "until"
    e <- exp_exp
    return $ SUntil e b
    
stmt_if = do
    reserved "if"
    e <- exp_exp
    reserved "then"
    b <- block
    eb <- many $ do
              reserved "elseif"
              e_ <- exp_exp
              reserved "then"
              b_ <- block
              return (e_, b_)
    df <- option Nothing $ do
              reserved "else"
              liftM Just $ block
    reserved "end"
    return $ SIf ((e, b):eb) df

stmt_return = do
    reserved "return"
    liftM SReturn $ option [] explist1

stmt_break = do
    reserved "break"
    return SBreak

stmt_for = do
    reserved "for"
    (ids, gen) <- (try stmt_for1 <|> stmt_for2)
    reserved "do"
    b <- block
    reserved "end"
    return $ SFor ids gen b
    where
       stmt_for1 = do
           id <- tok_identifier
           special "="
           from <- exp_exp
           special ","
           to <- exp_exp
           step <- option Nothing $ do
                       special ","
                       liftM Just exp_exp
           return $ ([id], ForNum from to step)
           
       stmt_for2 = do
           ids <- sepBy1 tok_identifier (special ",")
           reserved "in"
           exs <- explist1
           return $ (ids, ForIter exs)

stmt_function = do
    reserved "function"
    (n, a) <- funcname
    pos <- getPosition
    ref <- tolvar $ foldl EFieldRef 
                          (EVar $ head n) 
                          (map (EString pos) $ tail n)
    b <- funcbody
    case (a, b) of
        (False, _) ->
            return $ SAssignment [ref] [b]
        (True, EFunction pn va bb) ->
            return $ SAssignment [ref] [EFunction ("self":pn) va bb]

stmt_local = do
    reserved "local"
    do
        reserved "function" 
        n <- tok_identifier
        b <- funcbody
        return $ SLocalDef [n] [b]
      <|> do
        ns <- namelist1
        exs <- option [] $ do
                  special "="
                  explist1
        return $ SLocalDef ns exs

debug x = trace (show x) x

stmt_assignment_or_funccall = do
    ex <- primaryexp
    case ex of
        -- Function call
        (ECall _ _) -> return $ SAssignment [] [ex]
        (EMemberCall _ _ _) -> return $ SAssignment [] [ex]
        -- Assignment
        (EVar n) -> stmt_assignment' [LVar n]
        (EFieldRef t f) -> stmt_assignment' [LFieldRef t f]
        _ -> fail "Invalid statement"
        
stmt_assignment' lhs = 
    do
        special ","
        lv <- lvalue
        stmt_assignment' (lv:lhs)
      <|> do
        special "="
        vals <- explist1
        return $ SAssignment (reverse lhs) vals

-----------

lvalue = do
    ex <- primaryexp
    tolvar ex

tolvar ex = do
    case ex of
        (EVar n) -> return $ LVar n
        (EFieldRef t f) -> return $ LFieldRef t f
        _ -> fail "Invalid lvalue"

-----------

exp_anonfunction = do
    reserved "function"
    funcbody

funcbody = do
    (pn, vararg) <- parens parlist0
    b <- block
    reserved "end"
    return $ EFunction pn vararg b


funcname = do
    n' <- sepBy1 tok_identifier (special ".")
    do
        special ":"
        i <- tok_identifier
        return (n' ++ [i], True)
      <|>
        return (n', False)

namelist1 = sepBy1 tok_identifier (special ",")

explist1 = sepBy1 exp_exp (special ",")

-----------

prefixexp = choice [
    tok_identifier >>= return . EVar,
    parens exp_exp
    ]

primaryexp = do
    pfx <- prefixexp
    more pfx
    where
        more i = do { e <- dot_index i; more e }
             <|> do { e <- brace_index i; more e }
             <|> do { e <- member_call i; more e }
             <|> do { e <- fcall i; more e}
             <|> return i

        dot_index e = do
            special "."
            pos <- getPosition
            id <- tok_identifier
            return $ EFieldRef e (EString pos id)
            
        brace_index e = liftM (EFieldRef e) $ brackets exp_exp
        
        member_call e = do
            special ":"
            id <- tok_identifier
            arg <- funcargs
            return $ EMemberCall e id arg
            
        fcall e = liftM (ECall e) funcargs

simpleexp = choice [
    liftM ENumber $ tok_number,
    getPosition >>= \pos -> liftM (EString pos) $ tok_string,
    reserved "true" >> return (EBool True),
    reserved "false" >> return (EBool False),
    reserved "nil" >> return ENil,
    special "..." >> return EEllipsis,
    tableconstructor,
    exp_anonfunction,
    primaryexp
    ]

exp_exp =
    buildExpressionParser (map (map f) ops) simpleexp
    where
        f (s, True, _) = 
            Prefix (do{ operator s; return $ \e -> EUnOp s e})
        f (s, False, a) = 
            Infix (do{ operator s; return $ \e f -> EBinOp s e f}) a


ops = [
    [("^",   False,  AssocRight)],
    [("-",   True,   undefined),
     ("#",   True,   undefined),
     ("not", True,   undefined)],
    [("/",   False,  AssocLeft),
     ("*",   False,  AssocLeft)],
    [("-",   False,  AssocLeft),
     ("+",   False,  AssocLeft)],
    [("..",  False,  AssocRight)],
    [("==",  False,  AssocLeft),
     ("~=",  False,  AssocLeft),
     (">=",  False,  AssocLeft),
     ("<=",  False,  AssocLeft),
     (">",   False,  AssocLeft),
     ("<",   False,  AssocLeft)],
    [("and", False,  AssocLeft)],
    [("or",  False,  AssocLeft)]
    ]

-----------

funcargs = do
        (parens $ option [] explist1)
    <|> (liftM (:[]) $ tableconstructor)
    <|> (getPosition >>= \pos -> liftM (\s -> [EString pos s]) $ tok_string)

parlist0 = parlist1 <|> return ([], False) 

parlist1 =
    do  i <- tok_identifier
        (n, a) <- do { special ","; parlist1 } 
                  <|> return ([], False)
        return (i:n, a)
    <|>
    do  special "..."
        return ([], True)

-----------

tableconstructor = liftM ETableCons $ braces fieldlist

fieldlist = sepEndBy field fieldsep

field = do
    field_1 <|> field_2 <|> field_3
    where 
        field_1 = do
            f <- brackets exp_exp
            special "="
            v <- exp_exp
            return (Just f, v)
        field_2 = do 
            pos <- getPosition
            id <- try $ do i <- tok_identifier
                           special "="
                           return i
            v <- exp_exp
            return (Just (EString pos id), v)
        field_3 = do
            v <- exp_exp
            return (Nothing, v)

fieldsep = special "," <|> special ";"
