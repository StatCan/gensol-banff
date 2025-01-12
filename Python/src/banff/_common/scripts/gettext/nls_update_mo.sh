# nls_update_mo.sh
# Generates `.mo` file from `.po` file for all supported languages.  

source nls_variables.sh

# English

if [ -f ${po_path_en} ]; then
    msgfmt --output-file=${mo_path_en} ${po_path_en}
else
    echo "File does not exist: ${po_path_en}"
fi


# French
if [ -f ${po_path_fr} ]; then
    msgfmt --output-file=${mo_path_fr} ${po_path_fr}
else
    echo "File does not exist: ${po_path_fr}"
fi
