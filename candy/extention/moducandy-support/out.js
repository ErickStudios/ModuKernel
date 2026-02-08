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

function parseCode(code) {
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
      wordSymbol += c;
      continue;
    }

    // cierre de palabra
    if (wordSymbol.length > 0) {

    if (typeUsage) {
        typeUse = wordSymbol;
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
            }
          }

          typeUse = "";
      }
      if (wordSymbol === "return") {
          codeRet += "ret\n";
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
              codeRet += assemblyCode + "\n";
          }
      }
      else if (c === '/' && code[i + 1] === '/') {
        inComment = true;
        codeRet += ';';
        i++;
      }
      else if (c === '(' && code[i + 1] === ')') {
      i++;

      let upper = wordSymbol.toUpperCase();
      codeRet += "call " + candyVarToRegister(upper) + "\n";
      }
      else if (c === '<' && code[i + 1] === '=') {
      i += 2;

      let candyVar = "";
      while (isLetter(code[i])) {
          candyVar += code[i];
          i++;
      }

        console.log(variables);

      for (const v of variables) {
          if (v.name === candyVar) {
          codeRet +=
              "call DATA_MODUCANDY_DATA\n" +
              "mov ebx," + v.offset + "\n" +
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

      codeRet +=
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

      codeRet += "jg " + label.toUpperCase() + "\n";
      }
      else if (c === '+') {
      i++;

      let candyVar = "";
      while (isLetter(code[i])) {
          candyVar += code[i];
          i++;
      }

      codeRet +=
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

      codeRet +=
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

      codeRet +=
          "push eax\n" +
          "push edx\n" +
          "mov eax," + candyVarToRegister(wordSymbol) + "\n" +
          "cdq\n" +
          "idiv " + candyVarToRegister(candyVar) + "\n" +
          "mov " + candyVarToRegister(wordSymbol) + ",eax\n" +
          "pop edx\n" +
          "pop eax\n";
      }
      else if (c === '-' && code[i + 1] === '>') {
      i += 2;

      let candyVar = "";
      while (isLetter(code[i])) {
          candyVar += code[i];
          i++;
      }

      codeRet +=
          "pop " +
          candyVarToRegister(candyVar) +
          "\n";
      }
      else if (c === '-') {
      i++;

      let candyVar = "";
      while (isLetter(code[i])) {
          candyVar += code[i];
          i++;
      }

      codeRet +=
          "sub " +
          candyVarToRegister(wordSymbol) +
          "," +
          candyVarToRegister(candyVar) +
          "\n";
      }
      else if (c === '=') {
      i++;

      let candyVar = "";
      while (isLetter(code[i])) {
          candyVar += code[i];
          i++;
      }

      codeRet +=
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

      codeRet += "je " + label.toUpperCase() + "\n";
      }
    
      wordSymbol = "";
    }
  }

  return codeRet + dataSection + datasReal + datasFunctions;
}

module.exports = { candyVarToRegister,isLetter, VariableModuCandy, parseCode}