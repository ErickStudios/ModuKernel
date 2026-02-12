const vscode = require("vscode");
const { parseCode, parseCodeInternal, Enums } = require("./out"); // tu ModuCandy core

function buildSymbolTree(symbolsMap) {
  const tree = {};

  symbolsMap.forEach((value, key) => {
    const parts = key.split("::");
    let current = tree;

    for (let i = 0; i < parts.length; i++) {
      const part = parts[i];
      if (!current[part]) {
        current[part] = { __children: {}, __value: null };
      }
      if (i === parts.length - 1) {
        current[part].__value = value; // valor final
      }
      current = current[part].__children;
    }
  });

  return tree;
}
function activate(context) {

  const disposable = vscode.commands.registerCommand(
    "moducandy.convertToAsm",
    async () => {

      const editor = vscode.window.activeTextEditor;

      if (!editor) {
        vscode.window.showErrorMessage("No active editor");
        return;
      }

      const document = editor.document;

      // 1️⃣ verificar extensión
      if (!document.fileName.endsWith(".cdy")) {
        vscode.window.showWarningMessage("Current file is not a .cdy file");
        return;
      }

      // 2️⃣ leer código
      const sourceCode = document.getText();

      let asm;
      try {
        asm = parseCode(sourceCode);
      } catch (e) {
        vscode.window.showErrorMessage("ModuCandy error: " + e.message);
        return;
      }

      // 3️⃣ abrir Untitled con ASM
      const asmDoc = await vscode.workspace.openTextDocument({
        content: asm,
        language: "asm" // o "nasm"
      });

      await vscode.window.showTextDocument(asmDoc, {
        preview: false
      });
    }
  );
  // registrar proveedor de autocompletado con los Symbols
  const provider = vscode.languages.registerCompletionItemProvider({ language: "moducandy" }, // tu lenguaje
  {
    provideCompletionItems(document, position) {
  const result = parseCodeInternal(document.getText());
  const tree = buildSymbolTree(result.Symbols);

  // texto antes del cursor
  const prefix = document.getText(new vscode.Range(new vscode.Position(position.line, 0), position));
  const parts = prefix.split("::");

  // navegar el árbol según lo escrito
  let current = tree;
  for (let i = 0; i < parts.length - 1; i++) {
    if (current[parts[i]]) {
      current = current[parts[i]].__children;
    } else {
      return []; // no hay coincidencias
    }
  }

  // sugerir hijos del nivel actual
  const items = [];
  for (const name in current) {
    let Pref = parts.slice(0, -1);
    
    const pf = (Pref.length > 0 ? Pref.join("::") + "::" : "") + name;
    console.log(pf)

    const item = new vscode.CompletionItem(name, (result.Symbols.has(pf) ? (result.Symbols.get(pf) == Enums.TypeOfData.Function ? vscode.CompletionItemKind.Function : vscode.CompletionItemKind.Variable) : vscode.CompletionItemKind.Module));
    items.push(item);
  }

  return items;
}
  }
);

  context.subscriptions.push(provider);
  context.subscriptions.push(disposable);
}

function deactivate() {}

module.exports = {
  activate,
  deactivate
};
