const vscode = require("vscode");
const { parseCode } = require("./out"); // tu ModuCandy core

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

  context.subscriptions.push(disposable);
}

function deactivate() {}

module.exports = {
  activate,
  deactivate
};
