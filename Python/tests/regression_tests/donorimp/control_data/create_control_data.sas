/* Run this program to create all SAS "control data" datasets 
    - control_data_dir MUST be updated to point to the folder housing this file.  
    - must have access to Banff procedures
*/

%let control_data_dir=C:\git\banff_redesign\Banff\BanffProcedures\Python\banff\tests\regression_tests\donorimp\control_data;

%include "&control_data_dir\data_donorimp_r04_SAS.sas";
%include "&control_data_dir\data_donorimp_t14_SAS.sas";
%include "&control_data_dir\data_donorimp_a03_SAS.sas";