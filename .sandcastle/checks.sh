#!/usr/bin/env bash
set -euo pipefail

export SDL_VIDEODRIVER=dummy
export SDL_RENDER_DRIVER=software
export PYTHONDONTWRITEBYTECODE=1

case "${1:-core}" in
    core)
        checks=(
            tests/sv_arch_checks.py
            tests/sv_contact_checks.py
            tests/sv_contact_live_checks.py
            tests/sv_endpoint_checks.py
            tests/sv_hp_checks.py
            tests/sv_lifecycle_checks.py
            tests/sv_login_checks.py
            tests/sv_login_live_checks.py
            tests/sv_macro_checks.py
            tests/sv_message_checks.py
            tests/sv_metaserver_live_checks.py
            tests/sv_options_checks.py
            tests/sv_profile_checks.py
            tests/sv_request_checks.py
            tests/sv_runtime_checks.py
            tests/sv_settings_checks.py
            tests/sv_vault_checks.py
        )
        ;;
    registry)
        checks=(
            tests/sv_capabilities_checks.py
            tests/sv_checkpoint_checks.py
            tests/sv_evidence_checks.py
            tests/sv_html_checks.py
            tests/sv_reconciliation_checks.py
            tests/sv_review_checks.py
            tests/sv_runtime_producer_checks.py
        )
        ;;
    *) echo "Usage: bash checks.sh [core|registry]" >&2; exit 2 ;;
esac

for check in "${checks[@]}"; do
    printf '\n== %s ==\n' "$check"
    python3 -B "$check"
done

if [[ ${1:-core} == core ]]; then
    python3 -B tests/sv_shell_smoke.py --backend software
fi
