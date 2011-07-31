-- 
-- Copyright (c) Tuomo Valkonen 2006.
--
-- This program is free software; you can redistribute it and/or modify
-- it under the terms of the GNU General Public License as published by
-- the Free Software Foundation; either version 2 of the License, or
-- (at your option) any later version.
--

import System.IO.Error
import Control.Monad.State
import Control.Monad.Error
import System.Environment       ( getArgs, getProgName )
import System.Console.GetOpt
import System.Exit              ( exitWith, exitFailure, ExitCode(..) )
import System.IO                ( openFile, IOMode(..), hGetContents, 
                                  hClose, stderr, hPutStr )
import Data.List

import Kuu.Token                ( tokenize, filter_comments )
import Kuu.Parser               ( parse_chunk )
import Kuu.AST

data Extracted = Extracted SourcePos String deriving Eq

concatExtracted :: [Extracted] -> Extracted
concatExtracted [] = undefined
concatExtracted ((Extracted pos s) : rest) = Extracted pos (s ++ concatMap (\(Extracted _ s) -> s) rest)

-------

die s = do
    p <- getProgName
    mapM_ (hPutStr stderr) [p, ": ", s, "\n"]
    exitFailure

-------

e2 e blk = extract_expr e >> extract_blk blk

nop = return ()

-------

extract_lvalue (LVar _)           = nop
extract_lvalue (LFieldRef e e')   = extract_expr e >> extract_expr e'

extract_tc (m,  e)                = extract_expr e
                                    >> maybe nop extract_expr m
extract_expr (ECall e es)         = do_call e es
                                    
extract_expr (EFunction _ _ blk)  = extract_blk blk
extract_expr (EMemberCall e _ es) = mapM_ extract_expr (e:es)
extract_expr (ETableCons tc)      = mapM_ extract_tc tc
extract_expr (EUnOp _ e)          = extract_expr e
extract_expr (EBinOp _ e e')      = extract_expr e >> extract_expr e'
extract_expr (EFieldRef e e')     = extract_expr e >> extract_expr e'
extract_expr _                    = nop

extract_if (e, blk)               = e2 e blk

extract_forgen (ForNum e e' m)    = extract_expr e
                                    >> extract_expr e'
                                    >> maybe nop extract_expr m
extract_forgen (ForIter es)       = mapM_ extract_expr es

extract_stmt (SDo blk)            = extract_blk blk
extract_stmt (SWhile e blk)       = e2 e blk
extract_stmt (SUntil e blk)       = e2 e blk 
extract_stmt (SIf conds mblk)     = mapM_ extract_if conds
                                    >> maybe nop extract_blk mblk
extract_stmt (SReturn exps)       = mapM_ extract_expr exps
extract_stmt SBreak               = nop
extract_stmt (SFor _ fg blk)      = extract_forgen fg >> extract_blk blk
extract_stmt (SAssignment lv e)   = mapM_ extract_lvalue lv
                                    >> mapM_ extract_expr e
extract_stmt (SLocalDef _ es)     = mapM_ extract_expr es

extract_blk (Block ss)            = mapM_ extract_stmt ss

-------

do_call (EVar nm) ees@(e:es) = do
    names <- gets snd
    if nm `elem` names
        then do
            s <- do_extract e
            modify $ \(ss, n) -> (s:ss, n)
            mapM_ extract_expr es
          -- Remove to fail on invalid strings
          `catchError` (\e -> return ())
        else mapM_ extract_expr ees
        
do_call e es = 
    mapM_ extract_expr (e:es)

do_extract (EString pos s)            = return (Extracted pos s)
do_extract (EBinOp ".." e e')     = liftM concatExtracted $ mapM do_extract [e, e']
do_extract e                      = throwError ("Invalid string: " ++ show e)

-------

-- Extract strings in an error/state monad with state
-- (result, function names)
extract fname found names blk = 
    case execStateT (extract_blk blk) (found, names) of
        Left e -> die (fname ++ ": " ++ e)
        Right r -> (return . reverse . fst) r

proc_parsec_result (Left error) = die (show error)
proc_parsec_result (Right result) = return result

parse_extract keys found name content = do
    toks <- proc_parsec_result $ tokenize name content
    ast <- proc_parsec_result $ parse_chunk name (filter_comments toks)
    extract name found keys ast

open_parse_extract keys found name =
    readFile name >>= parse_extract keys found name

-------

format = ('\n':) . (concatMap format_string) . nub

format_string (Extracted pos s) = "#: " ++ sourceName pos ++ ":" ++ show (sourceLine pos) ++ 
                                  "\nmsgid " ++ show s ++ "\nmsgstr \"\"\n\n"

-------

data Opts = Help | OutFile String | Keyword String

options :: [OptDescr Opts]
options = [
    Option ['o']  ["output"]  (ReqArg OutFile "outfile") 
        "Output file",
    Option ['k']  ["keyword"] (ReqArg Keyword "keyword") 
        "Function to look for",
    Option ['h']  ["help"]    (NoArg Help)
        "Show this help"
    ]

usage = putStr $ usageInfo "Usage: lua-xgettext [option...] [file...]" options

do_opts v [] = return v
do_opts (outf, key) (o:oo) =
    case o of
        Help       -> usage >> exitWith ExitSuccess
        OutFile f  -> case outf of
                        Nothing -> do_opts (Just f, key) oo
                        Just _  -> die "Outfile set multiple times"
        Keyword k  -> do_opts (outf, k:key) oo

do_args args =
    case (getOpt Permute options args) of
        (o, inf, []) -> do
            (outf, keys) <- do_opts (Nothing, []) o
            return (inf, outf, keys)
        (_, _, errs) -> die (concat errs)

-------

main = do
    args <- getArgs
    (inf, outf, keys') <- do_args args
    let keys = if null keys' then ["_"] else keys'
    strings <- case inf of
        [] -> getContents >>= parse_extract keys [] "stdin"
        _  -> foldM (open_parse_extract keys) [] inf
    case outf of
        Nothing -> putStr (format strings)
        Just f -> writeFile f (format strings)
    