// dependencias
const fs =                                  require("fs");
const path =                                require("path");
const lib_path =                            path.join(__dirname, "library");
const json_config_path =                    path.join(lib_path, "dependy.json");
// variables
var activeJson =                            JSON.parse(fs.readFileSync(json_config_path, "utf-8"));
// clases
class PathDependy {constructor(logica_nsp, directory) {
    this.logica_nsp =                       logica_nsp;
    this.directory =                        directory;
}};
// funciones
/** @returns {PathDependy[]} */
function checkDependenceTree() {
    const results = [];

    // recorrer todas las claves del root
    for (const key of Object.keys(activeJson)) {
        const node = activeJson[key];

        // verificar si es objeto y tiene el atributo type correcto
        if (node && typeof node === "object" && node.type === "moducandy.module.tree") {
            // recorrer los módulos
            for (const moduleName of node.modules) {
                results.push(new PathDependy(
                    key  + "::" + moduleName, 
                    path.join(lib_path, node.path, moduleName.toLowerCase() + ".cdy")
                ));
            }
        }
    }

    return results;

}

module.exports = {checkDependenceTree,PathDependy};