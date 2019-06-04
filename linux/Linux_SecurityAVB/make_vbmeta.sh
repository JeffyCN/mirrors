#!/bin/sh
python avbtool add_hash_footer --image boot.img --partition_size 26214400 --partition_name boot --key testkey_psk.pem --algorithm SHA512_RSA4096
python avbtool add_hash_footer --image recovery.img --partition_size 26214400 --partition_name recovery --key testkey_psk.pem --algorithm SHA512_RSA4096
python avbtool make_vbmeta_image --public_key_metadata metadata.bin --include_descriptors_from_image boot.img --include_descriptors_from_image recovery.img --algorithm SHA256_RSA4096 --rollback_index 0 --key testkey_psk.pem  --output vbmeta.img
