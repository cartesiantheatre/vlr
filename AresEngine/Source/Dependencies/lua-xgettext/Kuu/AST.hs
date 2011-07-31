-- 
-- Copyright (c) Tuomo Valkonen 2006.
--
-- This program is free software; you can redistribute it and/or modify
-- it under the terms of the GNU General Public License as published by
-- the Free Software Foundation; either version 2 of the License, or
-- (at your option) any later version.
--

module Kuu.AST (
    Block(..),
    Expr(..),
    Stmt(..),
    ForGen(..),
    LValue(..),
    SourcePos,
    sourceName,
    sourceLine
) where

import Text.ParserCombinators.Parsec.Pos

data Block = Block [Stmt]
           deriving Show

type Name = String

data LValue = LVar Name
            | LFieldRef Expr Expr
            deriving Show

data Expr = ENumber Double
          | EString SourcePos String
          | EBool Bool
          | ENil
          | EEllipsis
          | EFunction [Name] Bool Block
          | ECall Expr [Expr]
          | EMemberCall Expr Name [Expr]
          | ETableCons [(Maybe Expr, Expr)]
          | EUnOp String Expr
          | EBinOp String Expr Expr
          | EFieldRef Expr Expr
          | EVar Name
          deriving Show
          
data Stmt = SDo Block
          | SWhile Expr Block
          | SUntil Expr Block
          | SIf [(Expr, Block)] (Maybe Block)
          | SReturn [Expr]
          | SBreak
          | SFor [Name] ForGen Block
          | SAssignment [LValue] [Expr]
          | SLocalDef [Name] [Expr]
          deriving Show

data ForGen = ForNum Expr Expr (Maybe Expr)
            | ForIter [Expr]
            deriving Show
