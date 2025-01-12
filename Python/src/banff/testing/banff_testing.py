import banff
from banff._common.src.testing.assert_helper import (
    PytestProcedure,
    _default_proc_kwargs,
    get_control_dataset_path,
)


class PytestDetermin(PytestProcedure):
    """Pytest helper function for Deterministic procedure."""

    def __init__(
            self,
            #### Unit test parameters
            pytest_capture          = None, # for the `capfd` which captures the console log
            sas_log_name            = None,
            msg_list_sas            = None,
            msg_list_sas_exact      = None,
            msg_list_contains       = None,
            msg_list_contains_exact = None,
            expected_error_count    = 0,
            expected_warning_count  = 0,
            rc_should_be_zero       = True,
            round_data              = None,
            drop_columns            = True,
            expected_outdata        = None,
            expected_outstatus      = None,

            #### Banff parameters
            # USER C code parameters
            accept_negative    = None,
            no_by_stats        = None,
            edits              = None,
            unit_id            = None,
            by                 = None,

            # USER dataset references
            indata             = None,
            instatus           = None,
            outdata            = None,
            outstatus          = None,

            # Fancy New Options
            presort             = None,
            prefill_by_vars     = None,
            # OPTIONS
            trace               = None,
            capture             = False,
            **kwargs,
    ):
        if msg_list_sas is None:
            msg_list_sas=[]
        if msg_list_sas_exact is None:
            msg_list_sas_exact=[]
        if msg_list_contains is None:
            msg_list_contains=[]
        if msg_list_contains_exact is None:
            msg_list_contains_exact=[]

        # Create Banff call
        print("############################# PROCEDURE LOG STARTS on next line #################################")
        try:
            self.banff_call = banff.determin(
                # USER C code parameters

                accept_negative    = accept_negative,
                no_by_stats        = no_by_stats,
                edits              = edits,
                unit_id            = unit_id,
                by                 = by,

                # USER dataset references
                indata             = get_control_dataset_path(indata),
                instatus           = get_control_dataset_path(instatus),
                outstatus          = outstatus,
                outdata            = outdata,

                # Fancy New Options
                presort             = presort,
                prefill_by_vars     = prefill_by_vars,

                # OPTIONS
                trace               = trace,
                capture             = capture,
                **(_default_proc_kwargs | kwargs),
            )
        except banff.exceptions.ProcedureCError as e:
            self.c_return_code = e.return_code
            self.ds_compare_list = []
        else:
            self.c_return_code = self.banff_call.rc
            # create list of associated actual and expected output datasets
            self.ds_compare_list = [
                [self.banff_call._outdata,   expected_outdata],
                [self.banff_call._outstatus, expected_outstatus],
            ]

        super().__init__(
            pytest_capture          = pytest_capture,
            sas_log_name            = sas_log_name,
            msg_list_sas            = msg_list_sas,
            msg_list_sas_exact      = msg_list_sas_exact,
            msg_list_contains       = msg_list_contains,
            msg_list_contains_exact = msg_list_contains_exact,
            expected_error_count    = expected_error_count,
            expected_warning_count  = expected_warning_count,
            rc_should_be_zero       = rc_should_be_zero,
            round_data              = round_data,
            drop_columns            = drop_columns,
            by                      = by,
        )

class PytestDonorimp(PytestProcedure):
    """Pytest helper function for Donor Imputation procedure."""

    def __init__(
            self,
            #### Unit test parameters
            pytest_capture          = None, # for the `capfd` which captures the console log
            sas_log_name            = None,
            msg_list_sas            = None,
            msg_list_sas_exact      = None,
            msg_list_contains       = None,
            msg_list_contains_exact = None,
            expected_error_count    = 0,
            expected_warning_count  = 0,
            rc_should_be_zero       = True,
            round_data              = None,
            drop_columns            = True,
            expected_outdata        = None,
            expected_outstatus      = None,
            expected_outdonormap    = None,
            expected_outmatching_fields = None,

            #### Banff parameters
            # USER C code parameters
            unit_id             = None,
            by                  = None,
            must_match          = None,
            data_excl_var       = None,
            rand_num_var        = None,
            random              = None,
            seed                = None,
            edits               = None,
            post_edits          = None,
            display_level       = None,
            accept_negative     = None,
            no_by_stats         = None,
            min_donors          = None,
            percent_donors      = None,
            n                   = None,
            eligdon             = None,
            n_limit             = None,
            mrl                 = None,
            # USER dataset references
            indata              = None,
            instatus            = None,
            outdata             = None,
            outstatus           = None,
            outdonormap         = None,
            outmatching_fields  = None,

            # Fancy New Options
            presort             = None,
            prefill_by_vars     = None,
            exclude_where_indata = None,
            # OPTIONS
            trace               = None,
            capture             = False,
            **kwargs,
    ):
        if msg_list_sas is None:
            msg_list_sas=[]
        if msg_list_sas_exact is None:
            msg_list_sas_exact=[]
        if msg_list_contains is None:
            msg_list_contains=[]
        if msg_list_contains_exact is None:
            msg_list_contains_exact=[]

        #Create Banff call
        print("############################# PROCEDURE LOG STARTS on next line #################################")
        try:
            self.banff_call = banff.donorimp(
                # USER C code parameters
                unit_id            = unit_id,
                by                 = by,
                must_match         = must_match,
                data_excl_var      = data_excl_var,
                rand_num_var       = rand_num_var,
                random             = random,
                seed               = seed,
                edits              = edits,
                post_edits         = post_edits,
                display_level      = display_level,
                accept_negative    = accept_negative,
                no_by_stats        = no_by_stats,
                min_donors         = min_donors,
                percent_donors     = percent_donors,
                n                  = n,
                eligdon            = eligdon,
                n_limit            = n_limit,
                mrl                = mrl ,

                # USER dataset references
                indata             = get_control_dataset_path(indata),
                instatus           = get_control_dataset_path(instatus),
                outdata            = outdata,
                outstatus          = outstatus,
                outdonormap        = outdonormap,
                outmatching_fields = outmatching_fields,

                # Fancy New Options
                presort             = presort,
                prefill_by_vars     = prefill_by_vars,
                exclude_where_indata = exclude_where_indata,

                # OPTIONS
                trace               = trace,
                capture             = capture,
                **(_default_proc_kwargs | kwargs),
            )
        except banff.exceptions.ProcedureCError as e:
            self.c_return_code = e.return_code
            self.ds_compare_list = []
        else:
            self.c_return_code = self.banff_call.rc
            self.ds_compare_list = [
                [self.banff_call._outdata,   expected_outdata],
                [self.banff_call._outstatus, expected_outstatus],
                [self.banff_call._outdonormap,  expected_outdonormap],
                [self.banff_call._outmatching_fields,  expected_outmatching_fields],
            ]

        super().__init__(
            pytest_capture          = pytest_capture,
            sas_log_name            = sas_log_name,
            msg_list_sas            = msg_list_sas,
            msg_list_sas_exact      = msg_list_sas_exact,
            msg_list_contains       = msg_list_contains,
            msg_list_contains_exact = msg_list_contains_exact,
            expected_error_count    = expected_error_count,
            expected_warning_count  = expected_warning_count,
            rc_should_be_zero       = rc_should_be_zero,
            round_data              = round_data,
            drop_columns            = drop_columns,
            by                      = by,
        )

class PytestEditstat(PytestProcedure):
    """Pytest helper function for Edit Statistics procedure."""

    def __init__(
            self,
            #### Unit test parameters
            pytest_capture          = None, # for the `capfd` which captures the console log
            sas_log_name            = None,
            msg_list_sas            = None,
            msg_list_sas_exact      = None,
            msg_list_contains       = None,
            msg_list_contains_exact = None,
            expected_error_count    = 0,
            expected_warning_count  = 0,
            rc_should_be_zero       = True,
            round_data              = None,
            drop_columns            = True,
            expected_outedit_applic     = None,
            expected_outedit_status     = None,
            expected_outglobal_status   = None,
            expected_outk_edits_status  = None,
            expected_outedits_reduced   = None,
            expected_outvars_role       = None,

            #### Banff parameters
            # USER C code parameters
            accept_negative    = None,
            edits              = None,
            by                 = None,
            # USER dataset references
            indata             = None,
            outedit_applic     = None,
            outedit_status     = None,
            outglobal_status   = None,
            outk_edits_status  = None,
            outedits_reduced   = None,
            outvars_role       = None,

            # Fancy New Options
            presort             = None,
            # OPTIONS
            trace               = None,
            capture             = False,
            **kwargs,
    ):
        if msg_list_sas is None:
            msg_list_sas=[]
        if msg_list_sas_exact is None:
            msg_list_sas_exact=[]
        if msg_list_contains is None:
            msg_list_contains=[]
        if msg_list_contains_exact is None:
            msg_list_contains_exact=[]

        #Create Banff call
        print("############################# PROCEDURE LOG STARTS on next line #################################")
        try:
                self.banff_call = banff.editstat(
                    # USER C code parameters
                    by                 = by,
                    edits              = edits,
                    accept_negative    = accept_negative,

                    # USER dataset references
                    indata             = get_control_dataset_path(indata),
                    outedit_applic     = outedit_applic,
                    outedit_status     = outedit_status,
                    outglobal_status   = outglobal_status,
                    outk_edits_status  = outk_edits_status,
                    outedits_reduced   = outedits_reduced,
                    outvars_role       = outvars_role,

                    # Fancy New Options
                    presort             = presort,

                    # OPTIONS
                    trace               = trace,
                    capture             = capture,
                    **(_default_proc_kwargs | kwargs),
                )
        except banff.exceptions.ProcedureCError as e:
            self.c_return_code = e.return_code
            self.ds_compare_list = []
        else:
            self.c_return_code = self.banff_call.rc
            self.ds_compare_list = [
                [self.banff_call._outedit_applic    , expected_outedit_applic,      {"drop_columns":None}],
                [self.banff_call._outedit_status    , expected_outedit_status,      {"drop_columns":None}],
                [self.banff_call._outglobal_status  , expected_outglobal_status,    {"drop_columns":None}],
                [self.banff_call._outk_edits_status , expected_outk_edits_status,   {"drop_columns":None}],
                [self.banff_call._outedits_reduced  , expected_outedits_reduced],
                [self.banff_call._outvars_role      , expected_outvars_role,        {"drop_columns":None}],
            ]

        super().__init__(
            pytest_capture          = pytest_capture,
            sas_log_name            = sas_log_name,
            msg_list_sas            = msg_list_sas,
            msg_list_sas_exact      = msg_list_sas_exact,
            msg_list_contains       = msg_list_contains,
            msg_list_contains_exact = msg_list_contains_exact,
            expected_error_count    = expected_error_count,
            expected_warning_count  = expected_warning_count,
            rc_should_be_zero       = rc_should_be_zero,
            round_data              = round_data,
            drop_columns            = drop_columns,
            by                      = by,
        )

class PytestErrorloc(PytestProcedure):
    """Pytest helper function for Error Localization procedure."""

    def __init__(
            self,
            #### Unit test parameters
            pytest_capture          = None, # for the `capfd` which captures the console log
            sas_log_name            = None,
            msg_list_sas            = None,
            msg_list_sas_exact      = None,
            msg_list_contains       = None,
            msg_list_contains_exact = None,
            expected_error_count    = 0,
            expected_warning_count  = 0,
            rc_should_be_zero       = True,
            round_data              = None,
            drop_columns            = True,
            expected_outstatus      = None,
            expected_outreject      = None,

            #### Banff parameters
            # USER C code parameters
            unit_id         = None,
            by              = None,
            rand_num_var    = None,
            edits           = None,
            weights         = None,
            cardinality     = None,
            time_per_obs    = None,
            seed            = None,
            display_level   = None,
            accept_negative = None,
            no_by_stats     = None,
            # USER dataset references
            indata          = None,
            instatus        = None,
            outstatus       = None,
            outreject       = None,

            # Fancy New Options
            presort             = None,
            prefill_by_vars     = None,
            # OPTIONS
            trace               = None,
            capture             = False,
            **kwargs,
    ):
        if msg_list_sas is None:
            msg_list_sas=[]
        if msg_list_sas_exact is None:
            msg_list_sas_exact=[]
        if msg_list_contains is None:
            msg_list_contains=[]
        if msg_list_contains_exact is None:
            msg_list_contains_exact=[]

        #Create Banff call
        print("############################# PROCEDURE LOG STARTS on next line #################################")
        try:
            self.banff_call = banff.errorloc(
                # USER C code parameters
                unit_id            = unit_id,
                by                 = by,
                rand_num_var       = rand_num_var,
                edits              = edits,
                weights            = weights,
                cardinality        = cardinality,
                time_per_obs       = time_per_obs,
                seed               = seed,
                display_level      = display_level,
                accept_negative    = accept_negative,
                no_by_stats        = no_by_stats,

                # USER dataset references
                indata             = get_control_dataset_path(indata),
                instatus           = get_control_dataset_path(instatus),
                outstatus          = outstatus,
                outreject          = outreject,

                # Fancy New Options
                presort             = presort,
                prefill_by_vars     = prefill_by_vars,

                # OPTIONS
                trace               = trace,
                capture             = capture,
                **(_default_proc_kwargs | kwargs),
            )
        except banff.exceptions.ProcedureCError as e:
            self.c_return_code = e.return_code
            self.ds_compare_list = []
        else:
            self.c_return_code = self.banff_call.rc
            self.ds_compare_list = [
                [self.banff_call._outstatus, expected_outstatus],
                [self.banff_call._outreject, expected_outreject],
            ]

        super().__init__(
            pytest_capture          = pytest_capture,
            sas_log_name            = sas_log_name,
            msg_list_sas            = msg_list_sas,
            msg_list_sas_exact      = msg_list_sas_exact,
            msg_list_contains       = msg_list_contains,
            msg_list_contains_exact = msg_list_contains_exact,
            expected_error_count    = expected_error_count,
            expected_warning_count  = expected_warning_count,
            rc_should_be_zero       = rc_should_be_zero,
            round_data              = round_data,
            drop_columns            = drop_columns,
            by                      = by,
        )

class PytestEstimato(PytestProcedure):
    """Pytest helper function for Estimator procedure."""

    def __init__(
            self,
            #### Unit test parameters
            pytest_capture          = None, # for the `capfd` which captures the console log
            sas_log_name            = None,
            msg_list_sas            = None,
            msg_list_sas_exact      = None,
            msg_list_contains       = None,
            msg_list_contains_exact = None,
            expected_error_count    = 0,
            expected_warning_count  = 0,
            rc_should_be_zero       = True,
            round_data              = None,
            drop_columns            = True,
            expected_outstatus      = None,
            expected_outdata        = None,
            expected_outacceptable  = None,
            expected_outest_ef      = None,
            expected_outest_lr      = None,
            expected_outest_parm    = None,
            expected_outrand_err    = None,

            #### Banff parameters
            # USER C code parameters
            unit_id             = None,
            by                  = None,
            data_excl_var       = None,
            hist_excl_var       = None,
            seed                = None,
            verify_specs        = None,
            accept_negative     = None,
            no_by_stats         = None,
            # USER dataset references
            indata              = None,
            instatus            = None,
            indata_hist         = None,
            inalgorithm         = None,
            inestimator         = None,
            instatus_hist       = None,
            outstatus           = None,
            outdata             = None,
            outacceptable       = None,
            outest_ef           = None,
            outest_lr           = None,
            outest_parm         = None,
            outrand_err         = None,

            # Fancy New Options
            presort             = None,
            prefill_by_vars     = None,
            exclude_where_indata = None,
            exclude_where_indata_hist = None,
            # OPTIONS
            trace               = None,
            capture             = False,
            **kwargs,
    ):
        if msg_list_sas is None:
            msg_list_sas=[]
        if msg_list_sas_exact is None:
            msg_list_sas_exact=[]
        if msg_list_contains is None:
            msg_list_contains=[]
        if msg_list_contains_exact is None:
            msg_list_contains_exact=[]

        #Create Banff call
        print("############################# PROCEDURE LOG STARTS on next line #################################")
        try:
            self.banff_call = banff.estimato(
                # USER C code parameters
                unit_id            = unit_id,
                by                 = by,
                data_excl_var      = data_excl_var,
                hist_excl_var      = hist_excl_var,
                seed               = seed,
                verify_specs       = verify_specs,
                accept_negative    = accept_negative,
                no_by_stats        = no_by_stats,

                # USER dataset references
                indata             = get_control_dataset_path(indata),
                instatus           = get_control_dataset_path(instatus),
                indata_hist        = get_control_dataset_path(indata_hist),
                inalgorithm        = get_control_dataset_path(inalgorithm),
                inestimator        = get_control_dataset_path(inestimator),
                instatus_hist      = get_control_dataset_path(instatus_hist),
                outstatus          = outstatus,
                outdata            = outdata,
                outacceptable      = outacceptable,
                outest_ef          = outest_ef,
                outest_lr          = outest_lr,
                outest_parm        = outest_parm,
                outrand_err        = outrand_err,

                # Fancy New Options
                presort             = presort,
                prefill_by_vars     = prefill_by_vars,
                exclude_where_indata = exclude_where_indata,
                exclude_where_indata_hist = exclude_where_indata_hist,
                # OPTIONS
                trace               = trace,
                capture             = capture,
                **(_default_proc_kwargs | kwargs),
            )
        except banff.exceptions.ProcedureCError as e:
            self.c_return_code = e.return_code
            self.ds_compare_list = []
        else:
            self.c_return_code = self.banff_call.rc
            self.ds_compare_list = [
                [self.banff_call._outstatus,     expected_outstatus],
                [self.banff_call._outdata,       expected_outdata],
                [self.banff_call._outacceptable, expected_outacceptable,    {"drop_columns":None}],
                [self.banff_call._outest_ef,     expected_outest_ef,        {"drop_columns":None}],
                [self.banff_call._outest_lr,     expected_outest_lr,        {"drop_columns":None}],
                [self.banff_call._outest_parm,   expected_outest_parm,      {"drop_columns":None}],
                [self.banff_call._outrand_err,   expected_outrand_err,      {"drop_columns":None}],
            ]

        super().__init__(
            pytest_capture          = pytest_capture,
            sas_log_name            = sas_log_name,
            msg_list_sas            = msg_list_sas,
            msg_list_sas_exact      = msg_list_sas_exact,
            msg_list_contains       = msg_list_contains,
            msg_list_contains_exact = msg_list_contains_exact,
            expected_error_count    = expected_error_count,
            expected_warning_count  = expected_warning_count,
            rc_should_be_zero       = rc_should_be_zero,
            round_data              = round_data,
            drop_columns            = drop_columns,
            by                      = by,
        )

class PytestMassimpu(PytestProcedure):
    """Pytest helper function for Massimputation procedure."""

    def __init__(
            self,
            #### Unit test parameters
            pytest_capture          = None, # for the `capfd` which captures the console log
            sas_log_name            = None,
            msg_list_sas            = None,
            msg_list_sas_exact      = None,
            msg_list_contains       = None,
            msg_list_contains_exact = None,
            expected_error_count    = 0,
            expected_warning_count  = 0,
            rc_should_be_zero       = True,
            round_data              = None,
            drop_columns            = True,
            expected_outdata        = None,
            expected_outstatus      = None,
            expected_outdonormap    = None,

            #### Banff parameters
            # USER C code parameters
            accept_negative    = None,
            no_by_stats        = None,
            random             = None,
            mrl                = None,
            percent_donors     = None,
            min_donors         = None,
            n_limit            = None,
            seed               = None,
            unit_id            = None,
            by                 = None,
            must_impute        = None,
            must_match         = None,
            # USER dataset references
            indata             = None,
            outdata            = None,
            outdonormap        = None,

            # Fancy New Options
            presort             = None,
            # OPTIONS
            trace               = None,
            capture             = False,
            **kwargs,
    ):
        if msg_list_sas is None:
            msg_list_sas=[]
        if msg_list_sas_exact is None:
            msg_list_sas_exact=[]
        if msg_list_contains is None:
            msg_list_contains=[]
        if msg_list_contains_exact is None:
            msg_list_contains_exact=[]

        #Create Banff call
        print("############################# PROCEDURE LOG STARTS on next line #################################")
        try:
            self.banff_call = banff.massimpu(
                # USER C code parameters
                accept_negative    = accept_negative,
                no_by_stats        = no_by_stats,
                random             = random,
                mrl                = mrl,
                percent_donors     = percent_donors,
                min_donors         = min_donors,
                n_limit            = n_limit,
                seed               = seed,
                unit_id            = unit_id,
                by                 = by,
                must_impute        = must_impute,
                must_match         = must_match,

                # USER dataset references
                indata             = get_control_dataset_path(indata),
                outdata            = outdata,
                outdonormap        = outdonormap,

                # Fancy New Options
                presort             = presort,
                # OPTIONS
                trace               = trace,
                capture             = capture,
                **(_default_proc_kwargs | kwargs),
            )
        except banff.exceptions.ProcedureCError as e:
            self.c_return_code = e.return_code
            self.ds_compare_list = []
        else:
            self.c_return_code = self.banff_call.rc
            self.ds_compare_list = [
                [self.banff_call._outdata,   expected_outdata],
                [self.banff_call._outstatus, expected_outstatus],
                [self.banff_call._outdonormap,  expected_outdonormap],
            ]

        super().__init__(
            pytest_capture          = pytest_capture,
            sas_log_name            = sas_log_name,
            msg_list_sas            = msg_list_sas,
            msg_list_sas_exact      = msg_list_sas_exact,
            msg_list_contains       = msg_list_contains,
            msg_list_contains_exact = msg_list_contains_exact,
            expected_error_count    = expected_error_count,
            expected_warning_count  = expected_warning_count,
            rc_should_be_zero       = rc_should_be_zero,
            round_data              = round_data,
            drop_columns            = drop_columns,
            by                      = by,
        )

class PytestOutlier(PytestProcedure):
    """Pytest helper function for Outlier procedure."""

    def __init__(
            self,
            #### Unit test parameters
            pytest_capture          = None, # for the `capfd` which captures the console log
            sas_log_name            = None,
            msg_list_sas            = None,
            msg_list_sas_exact      = None,
            msg_list_contains       = None,
            msg_list_contains_exact = None,
            expected_error_count    = 0,
            expected_warning_count  = 0,
            rc_should_be_zero       = True,
            round_data              = None,
            drop_columns            = True,
            expected_outstatus      = None,
            expected_outstatus_detailed = None,
            expected_outsummary     = None,

            #### Banff parameters
            # USER C code parameters
            unit_id             = None,
            weight              = None,
            by                  = None,
            var                 = None,
            with_var            = None,
            accept_negative     = None,
            no_by_stats         = None,
            accept_zero         = None,
            outlier_stats       = None,
            beta_e              = None,
            beta_i              = None,
            exponent            = None,
            mdm                 = None,
            mei                 = None,
            mii                 = None,
            start_centile       = None,
            min_obs             = None,
            method              = None,
            side                = None,
            sigma               = None,
            # USER dataset references
            indata              = None,
            indata_hist         = None,
            outstatus           = None,
            outstatus_detailed  = None,
            outsummary          = None,

            # Fancy New Options
            presort             = None,
            exclude_where_indata = None,
            # OPTIONS
            trace               = None,
            capture             = False,
            **kwargs,
    ):
        if msg_list_sas is None:
            msg_list_sas=[]
        if msg_list_sas_exact is None:
            msg_list_sas_exact=[]
        if msg_list_contains is None:
            msg_list_contains=[]
        if msg_list_contains_exact is None:
            msg_list_contains_exact=[]

        #Create Banff call
        print("############################# PROCEDURE LOG STARTS on next line #################################")
        try:
            self.banff_call = banff.outlier(
                # USER C code parameters
                unit_id             = unit_id,
                weight              = weight,
                by                  = by,
                var                 = var,
                with_var            = with_var,
                accept_negative     = accept_negative,
                no_by_stats         = no_by_stats,
                accept_zero         = accept_zero,
                outlier_stats       = outlier_stats,
                beta_e              = beta_e,
                beta_i              = beta_i,
                exponent            = exponent,
                mdm                 = mdm,
                mei                 = mei,
                mii                 = mii,
                start_centile       = start_centile,
                min_obs             = min_obs,
                method              = method,
                side                = side,
                sigma               = sigma,

                # USER dataset references
                indata              = get_control_dataset_path(indata),
                indata_hist         = get_control_dataset_path(indata_hist),
                outstatus           = outstatus,
                outstatus_detailed  = outstatus_detailed,
                outsummary          = outsummary,

                # Fancy New Options
                presort             = presort,
                exclude_where_indata = exclude_where_indata,
                # OPTIONS
                trace               = trace,
                capture             = capture,
                **(_default_proc_kwargs | kwargs),
            )
        except banff.exceptions.ProcedureCError as e:
            if isinstance(e.__cause__, OSError):
                raise
            self.c_return_code = e.return_code
            self.ds_compare_list = []
        else:
            self.c_return_code = self.banff_call.rc
            self.ds_compare_list = [
                [self.banff_call._outstatus,     expected_outstatus],
                [self.banff_call._outstatus_detailed,     expected_outstatus_detailed],
                [self.banff_call._outsummary,    expected_outsummary,   {"drop_columns":None}],
            ]

        super().__init__(
            pytest_capture          = pytest_capture,
            sas_log_name            = sas_log_name,
            msg_list_sas            = msg_list_sas,
            msg_list_sas_exact      = msg_list_sas_exact,
            msg_list_contains       = msg_list_contains,
            msg_list_contains_exact = msg_list_contains_exact,
            expected_error_count    = expected_error_count,
            expected_warning_count  = expected_warning_count,
            rc_should_be_zero       = rc_should_be_zero,
            round_data              = round_data,
            drop_columns            = drop_columns,
            by                      = by,
        )

class PytestProrate(PytestProcedure):
    """Pytest helper function for Prorate procedure."""

    def __init__(
            self,
            #### Unit test parameters
            pytest_capture          = None, # for the `capfd` which captures the console log
            sas_log_name            = None,
            msg_list_sas            = None,
            msg_list_sas_exact      = None,
            msg_list_contains       = None,
            msg_list_contains_exact = None,
            expected_error_count    = 0,
            expected_warning_count  = 0,
            rc_should_be_zero       = True,
            round_data              = None,
            drop_columns            = True,
            expected_outdata        = None,
            expected_outstatus      = None,
            expected_outreject      = None,

            #### Banff parameters
            # USER C code parameters
            accept_negative    = None,
            no_by_stats        = None,
            verify_edits       = None,
            lower_bound        = None,
            upper_bound        = None,
            decimal            = None,
            edits              = None,
            method             = None,
            modifier           = None,
            unit_id            = None,
            by                 = None,
            # USER dataset references
            indata             = None,
            instatus           = None,
            outstatus          = None,
            outdata            = None,
            outreject          = None,

            # Fancy New Options
            presort             = None,
            prefill_by_vars     = None,
            # OPTIONS
            trace               = None,
            capture             = False,
            **kwargs,
    ):
        if msg_list_sas is None:
            msg_list_sas=[]
        if msg_list_sas_exact is None:
            msg_list_sas_exact=[]
        if msg_list_contains is None:
            msg_list_contains=[]
        if msg_list_contains_exact is None:
            msg_list_contains_exact=[]

        #Create Banff call
        print("############################# PROCEDURE LOG STARTS on next line #################################")
        try:
            self.banff_call = banff.prorate(
                # USER C code parameters
                accept_negative    = accept_negative,
                no_by_stats        = no_by_stats,
                verify_edits       = verify_edits,
                lower_bound        = lower_bound,
                upper_bound        = upper_bound,
                decimal            = decimal,
                edits              = edits,
                method             = method,
                modifier           = modifier,
                unit_id            = unit_id,
                by                 = by,

                # USER dataset references
                indata             = get_control_dataset_path(indata),
                instatus           = get_control_dataset_path(instatus),
                outstatus          = outstatus,
                outdata            = outdata,
                outreject          = outreject,

                # Fancy New Options
                presort             = presort,
                prefill_by_vars     = prefill_by_vars,
                # OPTIONS
                trace               = trace,
                capture             = capture,
                **(_default_proc_kwargs | kwargs),
            )
        except banff.exceptions.ProcedureCError as e:
            self.c_return_code = e.return_code
            self.ds_compare_list = []
        else:
            self.c_return_code = self.banff_call.rc
            self.ds_compare_list = [
                [self.banff_call._outstatus, expected_outstatus],
                [self.banff_call._outdata,   expected_outdata],
                [self.banff_call._outreject, expected_outreject],
            ]

        super().__init__(
            pytest_capture          = pytest_capture,
            sas_log_name            = sas_log_name,
            msg_list_sas            = msg_list_sas,
            msg_list_sas_exact      = msg_list_sas_exact,
            msg_list_contains       = msg_list_contains,
            msg_list_contains_exact = msg_list_contains_exact,
            expected_error_count    = expected_error_count,
            expected_warning_count  = expected_warning_count,
            rc_should_be_zero       = rc_should_be_zero,
            round_data              = round_data,
            drop_columns            = drop_columns,
            by                      = by,
        )

class PytestVerifyed(PytestProcedure):
    """Pytest helper function for Verify Edits procedure."""

    def __init__(
            self,
            #### Unit test parameters
            pytest_capture          = None, # for the `capfd` which captures the console log
            sas_log_name            = None,
            msg_list_sas            = None,
            msg_list_sas_exact      = None,
            msg_list_contains       = None,
            msg_list_contains_exact = None,
            expected_error_count    = 0,
            expected_warning_count  = 0,
            rc_should_be_zero       = True,

            #### Banff parameters
            # USER C code parameters
            accept_negative    = None,
            extremal           = None,
            imply              = None,
            edits              = None,

            # OPTIONS
            trace               = None,
            capture             = False,
            **kwargs,
    ):
        if msg_list_sas is None:
            msg_list_sas=[]
        if msg_list_sas_exact is None:
            msg_list_sas_exact=[]
        if msg_list_contains is None:
            msg_list_contains=[]
        if msg_list_contains_exact is None:
            msg_list_contains_exact=[]

        #Create Banff call
        print("############################# PROCEDURE LOG STARTS on next line #################################")
        try:
            self.banff_call = banff.verifyed(
                # USER C code parameters
                accept_negative    = accept_negative,
                extremal           = extremal,
                imply              = imply,
                edits              = edits,

                # OPTIONS
                trace               = trace,
                capture             = capture,
                **(_default_proc_kwargs | kwargs),
            )
        except banff.exceptions.ProcedureCError as e:
            self.c_return_code = e.return_code
            self.ds_compare_list = []
        else:
            self.c_return_code = self.banff_call.rc
            self.ds_compare_list = []

        super().__init__(
            pytest_capture          = pytest_capture,
            sas_log_name            = sas_log_name,
            msg_list_sas            = msg_list_sas,
            msg_list_sas_exact      = msg_list_sas_exact,
            msg_list_contains       = msg_list_contains,
            msg_list_contains_exact = msg_list_contains_exact,
            expected_error_count    = expected_error_count,
            expected_warning_count  = expected_warning_count,
            rc_should_be_zero       = rc_should_be_zero,
            drop_columns            = False,
        )
