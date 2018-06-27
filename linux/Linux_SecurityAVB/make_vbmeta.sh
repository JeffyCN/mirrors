#!/bin/sh
python avbtool add_hash_footer --image boot.img --partition_size 9437184 --partition_name boot --key testkey_atx_psk.pem --algorithm SHA512_RSA4096
python avbtool make_vbmeta_image --public_key_metadata atx_metadata.bin --include_descriptors_from_image boot.img --algorithm SHA256_RSA4096 --rollback_index 0 --key testkey_atx_psk.pem  --output vbmeta.img
