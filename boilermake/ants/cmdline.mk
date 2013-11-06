CMDLINE_VAR_INFO_FILE=cmdline_var_info.txt
CMDLINE_FILE=.antsrc

${DIR}/cmdline.cpp : ${DIR}/cmdline.hpp
	echo "CMDLINE_VAR_INFO_FILE = ${CMDLINE_VAR_INFO_FILE}"

${DIR}/cmdline.hpp : $(CONFIG_VAR_INFO_FILE)
	cd ants && python ../utils/generate_cmdline.py $(CMDLINE_VAR_INFO_FILE) $(CMDLINE_FILE)
