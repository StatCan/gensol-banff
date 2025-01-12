# Native Language Support (NLS)

## Overview

To provide a bilingual Python package, two things must be taken care of:

- options must be provided to choose the display language (or automatically detect it)
- a framework implemented to display output in the selected language

Detection and provision of options are implemented using custom code as no obvious premade solution was found, so this is implemented in-house.  

For the translation framework, we employ [Python's `gettext` package](https://docs.python.org/3/library/gettext.html).  It is based off [GNU gettext](https://www.gnu.org/software/gettext/), a mature, seemingly well supported, and somewhat programming-language-independent framework for facilitating internationalization (*`i18n`*) and localization (*`l11n`*).  

> **NOTE**: `gettext` for C code  
> While it is possible that we could use this in C code as well, the effort required appears too great considering how well the current in-house method works

Implementation involved:

- adding a bit of supporting code
- marking messages that require translation
- encoding translations into a binary file
- including the binary translation file(s) when distributing the package

### Supporting Tools

`gettext` requires us to encode our translations in binary files which must be included with the distributed code.  The *GNU gettext utilities* provided for generating these files.  

- [xgettext](https://www.gnu.org/software/gettext/manual/html_node/xgettext-Invocation.html) looks at source code and builds a template text file (a `.pot` file) containing all messages marked for translation
- [msginit](https://www.gnu.org/software/gettext/manual/html_node/msginit-Invocation.html) takes the template and creates language-specific text file (a `.po` file) containing all messages, each with a spot where translations can be inserted
- [msgmerge](https://www.gnu.org/software/gettext/manual/html_node/msgmerge-Invocation.html) is used to update existing `.po` files when in-source messages are modified and a new `.pot` file is generated, keeping existing translations
- [msgfmt](https://www.gnu.org/software/gettext/manual/html_node/msgfmt-Invocation.html) converts each language-specific `.po` file into a binary file (a `.mo` file) which is then distributed with the software

> *Python Specific Tools*  
> Python installations may include a Python specific version of these tools (`pygettext.py`, `msgfmt.py`), however these tools critically do not include the `msgmerge` functionality listed above, so don't use these.  

## Implementation Process

### Supporting Code

We import Python's standard `gettext` package.  In our case, we'll use the "[Class based API](https://docs.python.org/3/library/gettext.html#class-based-api)" as recommended in the documentation.  We'll use some of its functions to configure available translations and select which language to display at runtime.  

> This is implemented in the `banff.nls` module

### Prepare/Mark Messages

Using this framework, the full original messages are left in the source files as-is, aside from being "*marked*" ("*wrapped*") in a special function which will allow `xgettext` to find messages in source and facilitate translation at runtime.  As is common convention, that function is named `_()`.  

For example, here is a marked message

```python
log_lcl.debug(_("Loading csv file into Pandas DataFrame"))
```

And here is a marked message which takes parameters

```python
log_lcl.debug(_("file exists: {}").format(dataset_ref))
```

- **NOTE:** using `"".format()` is preferred over `f""` as the latter would result in variable names being included in the generated `.pot` and `.po` files, likely preventing a single translation from being referenced in multiple locations

### Prepare Translations

#### Get language-specific `.po` files

This can be done with the script `Python/scripts/gettext/nls_update_po.sh`.  
This script is called in the pipeline job `nls_update_po`, which stores the template and the French and English `.po` files as artifacts.  This job should get executed whenever the pipeline changes include any Python source code.  

> **English `.po` File**
> The English `.po` file is handled differently than non-English files because we use full-text English messages in the source code (`log.error("file does not exist")`), as opposed to using a message ID like (`M123` or `ERROR_file_no_exist`).  
> Therefore, the `msginit` program creates an English `.po` file where the translation of each message is equal to the in-source message.  

The `.po` file will begin with some header information and subsequently lists each message.  

> **sample message**: "*file does not exist: \<file-name\>*"
>
> ```plaintext
> #: Python\src\banff\io_util\io_util.py:78
> msgid "file does not exist: {}"
> msgstr ""
> ```
>
> - `#:`  comment, indicate where the message was found in the source
> - `msgid`: the extracted message
> - `msgstr`: placeholder for translation

#### Populate With Translations

For non-English `.po` files, each messages `msgstr` should be populated with the correct translation.  

> **IMPORTANT**: file encoding
> The `.po` file's header defines the character set used.  
> example: `"Content-Type: text/plain; charset=cp1252\n"`
> Ensure that this matches the file's actual encoding, otherwise special characters (like `�`) will not appear properly at runtime.  

### Compile/Build Translations

Once populated with translations, create a `.mo` file from each `.po` file using the script `Python/scripts/gettext/nls_update_mo.sh`.  
This script is called in the pipeline job `nls_update_mo`, which stores the language-specific `.mo` files as artifacts.  This job should get executed whenever a pipeline detects changes to the language-specific `.po` files.  

### Publish

Finally, commit these `.mo` files to the repository from where they can be distributed with the package in subsequent builds.  

Note that we could build `.mo` files as part of the normal build process and never commit them to the repository.  By keeping this as a separate step and committing `.mo` files to version control we ensure developers always have a working copy of the translations, which is helpful when debugging locally (often on Windows, where it's harder to get the GNU gettext utilities).  
