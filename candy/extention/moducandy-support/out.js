const {PathDependy, checkDependenceTree} = require("./dependy.js")
const fs = require("fs");

class Enums {
  static TypeOfData = {
    Function: 0,
    Variable: 1
  };
}

function candyVarToRegister(candyVar) {
  switch (candyVar) {
    case "r1": return "eax";
    case "r2": return "ecx";
    case "r3": return "edx";
    case "r4": return "ebx";
    case "r5": return "esp";
    case "r6": return "ebp";
    case "r7": return "esi";
    case "r8": return "edi";
    default: return candyVar;
  }
}

function isLetter(c) {
  return (
    (c >= 'A' && c <= 'Z') ||
    (c >= 'a' && c <= 'z') ||
    (c >= '0' && c <= '9') ||
    c === '_' ||
    c === '[' ||
    c === ']' ||
    c === ':' ||
    c === '.'
  );
}

class VariableModuCandy {
  constructor(name, type, offset) {
    this.name = name;
    this.type = type;
    this.offset = offset;
  }
}

function parseCodeInternal(code) {
  /** @type { Map<string, number>} */
  let Symbols = new Map();

  let Tabulators = "   ";

  let codeRet = "";
  let wordSymbol = "";
  let typeUse = "";

  let dataSection =
`DATA_MODUCANDY_DATA:
   call DATA_MODUCANDY_DATA_GETEIP
   ret
   ;datas
`;

  let datasReal = "";
  let datasFunctions =
`; ModuCandy: eip get runtime for data calc
DATA_MODUCANDY_DATA_GETEIP:
   pop eax
   push eax
   inc eax
   ret
`;

  let variables = [];
  let offsetActual = 0;

  let inComment = false;
  let typeUsage = false;

  for (let i = 0; i < code.length; i++) {
    const c = code[i];

    // comentario
    if (inComment) {
      codeRet += c;
      if (c === '\n') inComment = false;
      continue;
    }

    // acumulación de símbolo
    if (isLetter(c)) {
      if (wordSymbol.length === 0 && c >= 'A' && c <= 'Z') {
        typeUsage = true;
      }

      if (typeUsage)
      {
        typeUse += c;
      }
      wordSymbol += c;
      continue;
    }

    // cierre de palabra
    if (typeUsage) {
        typeUsage = false;
      }
      else if (typeUse !== "") {
          // fn
          if (wordSymbol === "fn") {
            i++;
            while (code[i] === ' ' || code[i] === '\t') i++;

            let funcName = "";
            while (isLetter(code[i])) {
              funcName += code[i++];
            }

            Symbols.set(funcName, Enums.TypeOfData.Function);
            codeRet += `; type=${typeUse}\n${funcName}:\n`;
          }

          // let
          else if (wordSymbol === "let") {
            i++;
            while (code[i] === ' ' || code[i] === '\t') i++;

            let varName = "";
            while (isLetter(code[i])) {
              varName += code[i++];
            }

            let compileAsStru = false;
            for (let av of variables)
            {
              let a = typeUse;
              a += "::__New__";

              if (av.name == a)
              {
                compileAsStru = true;
                for (let av2 of variables) {
                  let str2 = av2.name;
                  let str1 = typeUse + "::";
                  let len = str1.length;
                  let comparate = str2.startsWith(str1) ? 0 : -1;

                    console.log("str2:", str2);
                    console.log("str1:", str1);
                    console.log("len:", len);

                  if (comparate == 0)
                  {
                    str2 = str2.substring(len);
                    // simular el ret ;place for a struct field
                    for (let i = 0; i < (av2.type === "u32" ? 4 : ( av2.type == "stru" ? 0 : 1)); i++) {
                        datasReal += "   ret ;place for a struct field\n";
                    }

                    let Variable = new VariableModuCandy(varName + "::" + str2, av2.type, offsetActual);

                    offsetActual += (av2.type === "u32" ? 4 : ( av2.type == "stru" ? 0 : 1));
                    Symbols.set(Variable.name, Enums.TypeOfData.Variable);
                    variables.push(Variable);
                  }
                }
                break;
              }
            }
            if (compileAsStru == false) {
              Symbols.set(varName, Enums.TypeOfData.Variable);
            if (typeUse === "BuiltIn_u32" || typeUse === "BuiltIn_i32") {
              for (let a = 0; a < 4; a++) {
                datasReal += "   ret ;place for variable (u/i32)\n";
              }

              variables.push(
                new VariableModuCandy(varName, "u32", offsetActual)
              );
              offsetActual += 4;
            }
            else if (typeUse === "BuiltIn_u8" || typeUse === "BuiltIn_i8") {
              datasReal += "   ret ;place for variable (u/i8)\n";

              variables.push(
                new VariableModuCandy(varName, "u8", offsetActual)
              );
              offsetActual += 1;
            }}
          }

          typeUse = "";
      }
      if (wordSymbol === "return") {
          codeRet += Tabulators + "ret\n";
      }
      else if (wordSymbol == "stru") {
          i++;
        let candyVar = "";
        while (isLetter(code[i])) {
            candyVar += code[i];
            i++;
        }
        Symbols.set(candyVar + "::__New__", Enums.TypeOfData.Variable);
        variables.push(new VariableModuCandy(candyVar + "::__New__", "stru", 0));

      }
      else if (wordSymbol === "candy") {
        i++;
        let candyVar = "";
        while (isLetter(code[i])) {
            candyVar += code[i];
            i++;
        }

        try {
          let deptree = checkDependenceTree();
          
          for (const dep of deptree) {
            if (dep.logica_nsp == candyVar)
            {
              let depa = parseCodeInternal(fs.readFileSync(dep.directory, "utf-8"));
              codeRet += depa.codeRet;
              Symbols = new Map([...Symbols, ...depa.Symbols]);
              break;
            }
          }
        } catch (error) {
        }
      }
      else if (wordSymbol === "unsafe") {
          i++;
          if (code[i] === '"') {
              let assemblyCode = "";
              i++;
              while (code[i] !== '"' && i < code.length) {
              assemblyCode += code[i];
              i++;
              }
              codeRet += Tabulators + assemblyCode + "\n";
          }
      }
      else if (c === '/' && code[i + 1] === '/') {
        inComment = true;
        codeRet += Tabulators + ';';
        i++;
      }
      else if (c === '(' && code[i + 1] === ')') {
      i++;

      let upper = wordSymbol.toUpperCase();
      codeRet += Tabulators + "call " + candyVarToRegister(upper) + "\n";
      }
      else if (c === '<' && code[i + 1] === '=') {
      i += 2;

      let candyVar = "";
      while (isLetter(code[i])) {
          candyVar += code[i];
          i++;
      }

      for (const v of variables) {
          if (v.name === candyVar) {
          codeRet += Tabulators +
              "call DATA_MODUCANDY_DATA\n" + Tabulators +
              "mov ebx," + v.offset + "\n" + Tabulators +
              "add eax,ebx\n";
          }
      }
      }
      else if (c === '?') {
      i++;

      let candyVar = "";
      while (isLetter(code[i])) {
          candyVar += code[i];
          i++;
      }

      codeRet += Tabulators +
          "cmp " +
          candyVarToRegister(wordSymbol) +
          "," +
          candyVarToRegister(candyVar) +
          "\n";
      }
      else if (c === '>' && wordSymbol === "") {
      i++;

      let label = "";
      while (isLetter(code[i])) {
          label += code[i];
          i++;
      }

      codeRet += Tabulators + "jg " + label.toUpperCase() + "\n";
      }
        else if (c === '<' && wordSymbol === "") {
      i++;

      let label = "";
      while (isLetter(code[i])) {
          label += code[i];
          i++;
      }

      codeRet += Tabulators + "jl " + label.toUpperCase() + "\n";
      }
      else if (c === '+') {
      i++;

      let candyVar = "";
      while (isLetter(code[i])) {
          candyVar += code[i];
          i++;
      }

      codeRet += Tabulators +
          "add " +
          candyVarToRegister(wordSymbol) +
          "," +
          candyVarToRegister(candyVar) +
          "\n";
      }
      else if (c === '*') {
      i++;

      let candyVar = "";
      while (isLetter(code[i])) {
          candyVar += code[i];
          i++;
      }

      codeRet += Tabulators +
          "SimpleMultiplication " +
          candyVarToRegister(wordSymbol) +
          "," +
          candyVarToRegister(candyVar) +
          "\n";
      }
      else if (c === '/') {
      i++;

      let candyVar = "";
      while (isLetter(code[i])) {
          candyVar += code[i];
          i++;
      }

      codeRet += Tabulators +
          "push eax\n" + Tabulators +
          "push edx\n" + Tabulators +
          "mov eax," + candyVarToRegister(wordSymbol) + "\n" + Tabulators +
          "cdq\n" + Tabulators +
          "idiv " + candyVarToRegister(candyVar) + "\n" + Tabulators +
          "mov " + candyVarToRegister(wordSymbol) + ",eax\n" + Tabulators +
          "pop edx\n" + Tabulators +
          "pop eax\n";
      }
      else if (c === '-' && code[i + 1] === '>') {
      i += 2;

      let candyVar = "";
      while (isLetter(code[i])) {
          candyVar += code[i];
          i++;
      }

      codeRet += Tabulators +
          "pop " +
          candyVarToRegister(candyVar) +
          "\n";
      }
      else if ((c === '<' && code[i + 1] === '-') && wordSymbol !== "") {
      i += 2;

      codeRet += Tabulators +
          "push " +
          candyVarToRegister(wordSymbol) +
          "\n";
      }
      else if (c === '-') {
      i++;

      let candyVar = "";
      while (isLetter(code[i])) {
          candyVar += code[i];
          i++;
      }

      codeRet += Tabulators +
          "sub " +
          candyVarToRegister(wordSymbol) +
          "," +
          candyVarToRegister(candyVar) +
          "\n";
      }
      else if (c === '=' && wordSymbol !== "") {
      i++;

      let candyVar = "";
      while (isLetter(code[i])) {
          candyVar += code[i];
          i++;
      }

      codeRet += Tabulators +
          "mov " +
          candyVarToRegister(wordSymbol) +
          "," +
          candyVarToRegister(candyVar) +
          "\n";
      }
      else if (c === '=' && wordSymbol === "") {
      i++;

      let label = "";
      while (isLetter(code[i])) {
          label += code[i];
          i++;
      }

      codeRet += Tabulators + "je " + label.toUpperCase() + "\n";
      }
    
      wordSymbol = "";
  }

  
  return {codeRet: codeRet , dataSection: dataSection , datasReal: datasReal, datasFunctions: datasFunctions, final:codeRet+dataSection+datasReal+datasFunctions , Symbols:Symbols};
}

function parseCode(code)
{
  let abc = parseCodeInternal(code);
  console.log(abc);
  return abc.final;
}

module.exports = { candyVarToRegister,isLetter, VariableModuCandy, parseCode, parseCodeInternal, Enums}