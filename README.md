# Zax Compiler

This is the prototype compiler to parse and generate output for the [Zax](https://zax.io/) language.


## Status

Early coding stages.

The first goal is to make the code function without concerning too much about performance, instance ownerships, or optimal coding practices. The generator will output C or C++ for the first pass and later integrate LLVM to perform direct compilations.


## Dependencies

The following dependencies are required:
* [nlohmann/json](https://github.com/nlohmann/json)
* [Microsoft GSL](https://github.com/microsoft/GSL)
* [SafeInt](https://github.com/dcleblanc/SafeInt)
* [zs](https://github.com/robin-raymond/zs)

These dependencies are expected to be cloned within the same containing folder that already contains [Zax Compiler](https://github.com/robin-raymond/zax-compiler).


## Building

Currently only the Microsoft Visual Studio Compiler is supported and the editor / compiler using Visual Studio Code with the C/C++ for [Visual Studio Code](https://marketplace.visualstudio.com/items?itemName=ms-vscode.cpptools). The code is designed cross platform thus other compiler options can be made available.

See [https://code.visualstudio.com/docs/languages/cpp](https://code.visualstudio.com/docs/languages/cpp).

Important quick guide steps:
* Install [Visual Studio Community 2019](https://visualstudio.microsoft.com/vs/)
* Install [Visual Studio Code](https://code.visualstudio.com/download)
* Install [C/C++ for Visual Studio Code](https://marketplace.visualstudio.com/items?itemName=ms-vscode.cpptools)
* Launch a [Developer Command Prompt for VS2019](https://docs.microsoft.com/en-us/dotnet/framework/tools/developer-command-prompt-for-vs)
* `cd E:\sources\zax-compiler` (change as appropriate)
* Launch Visual Studio Code from the prompt `code .`
* Select `Terminal->Run Build Task...`  (some option choice may be prompted)


## License

MIT License. See [LICENSE](https://github.com/robin-raymond/zax-compiler/blob/master/LICENSE).

Please examine dependencies for their licensing.


## Issues

As this is early stages, there is no support offered at this time. The compiler will not do anything at this phase of coding.

Please post [issues on GitHub](https://github.com/robin-raymond/zax-compiler/issues).
