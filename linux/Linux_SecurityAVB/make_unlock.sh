#!/bin/sh
python avb-challenge-verify.py raw_atx_unlock_challenge.bin atx_product_id.bin
python avbtool make_atx_unlock_credential --output=atx_unlock_credential.bin --intermediate_key_certificate=atx_pik_certificate.bin --unlock_key_certificate=atx_puk_certificate.bin --challenge=atx_unlock_challenge.bin --unlock_key=testkey_atx_puk.pem
