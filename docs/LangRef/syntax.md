# Syntax

## 

# EBNF
## program_stream
`program_stream = { ( compound_def | term ) ( "END" | "." ) }`
## compound_def
```
compound_def = 
  "MODULE" atom compound_def |
  ( "PRIVATE" | "HIDE" ) defsequence compound_def |
  ( "PUBLIC" | "LIBRA" | "IN" ) defsequence
```
## defsequence
`defsequence = definition { ";" definition }`
## definition
`definition = [ compound_def ( "END" | "." ) ] [ atom "==" term ]`
## term
`term = { factor } `
## factor
`factor = `