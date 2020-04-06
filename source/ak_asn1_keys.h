/* ----------------------------------------------------------------------------------------------- */
/*  Copyright (c) 2020 by Axel Kenzo, axelkenzo@mail.ru                                            */
/*                                                                                                 */
/*  Файл ak_asn1_keys.h                                                                            */
/*  - содержит описания функций, предназначенных для экспорта/импорта ключевой информации          */
/* ----------------------------------------------------------------------------------------------- */
#ifndef __AK_ASN1_KEYS_H__
#define __AK_ASN1_KEYS_H__

/* ----------------------------------------------------------------------------------------------- */
 #include <ak_asn1.h>
 #include <ak_sign.h>

/* ----------------------------------------------------------------------------------------------- */
/** \addtogroup backend_keys Функции внутреннего интерфейса. Управление ключами.
 * @{*/
/* ----------------------------------------------------------------------------------------------- */
/*! \brief Функция вырабатывает производные ключи шифрования и имитозащиты контента из пароля и
    экспортирует в ASN.1 дерево параметры ключа, необходимые для восстановления. */
 int ak_asn1_context_add_derived_keys_from_password( ak_asn1 , ak_oid , ak_bckey ,
                                                          ak_bckey , const char * , const size_t );
/*! \brief Функция восстанавливает производные ключи шифрования и имитозащиты на основе информации,
   хранящейся в ASN.1 дереве. */
 int ak_asn1_context_get_derived_keys( ak_asn1 , ak_bckey , ak_bckey );

/* ----------------------------------------------------------------------------------------------- */
/*! \brief Функция проверяет, что данный узел ASN.1 дерева является контейнером. */
 bool_t ak_tlv_context_check_libakrypt_container( ak_tlv tlv, ak_asn1 * , ak_asn1 * );
/*! \brief Функция возвращает тип контента, помещенного в ASN.1 контейнер. */
 crypto_content_t ak_asn1_context_get_content_type( ak_asn1 );
/*! \brief Функция получает служебную информацию о ключе, расположенном в ASN.1 контейнере. */
 int ak_asn1_context_get_symmetric_key_info( ak_asn1 ,
                                      ak_oid * , ak_pointer * , size_t * , char ** , ak_resource );
/*! \brief Функция инициализирует секретный ключ значениями, расположенными в ASN.1 контейнере. */
 int ak_asn1_context_get_skey( ak_asn1 , ak_skey , ak_bckey , ak_bckey );

/* ----------------------------------------------------------------------------------------------- */
/*! \brief Функция экспортирует секретный ключ криптографического преобразования в ASN.1 дерево
   с использованием пользовательского пароля. */
 int ak_key_context_export_to_asn1_with_password( ak_pointer , oid_engines_t ,
                                            ak_asn1 , const char * , const size_t , const char * );
/*! \brief Функция экспортирует секретный ключ в заданный файл в виде der-последовательности. */
 int ak_key_context_export_to_derfile_with_password( ak_pointer , oid_engines_t ,
                               const char *, const size_t , const char * , char * , const size_t );

/* ----------------------------------------------------------------------------------------------- */
/*! \brief Функция импортирует ключ симметричного криптографического преобразования из
   der-последовательности, хранящейся в заданном файле. */
 int ak_symmetric_key_context_import_from_derfile( ak_pointer , oid_engines_t ,
                                                                          const char * , char ** );
/*! \brief Функция импортирует ключ алгоритма блочного шифрования из заданного файла. */
 int ak_bckey_context_import_from_derfile( ak_bckey , const char * , char ** );
/*! \brief Функция импортирует ключ алгоритма блочного шифрования из заданного файла. */
 int ak_hmac_context_import_from_derfile( ak_hmac , const char * , char ** );

/** @}*/
#endif
/* ----------------------------------------------------------------------------------------------- */
/*                                                                                 ak_asn1_keys.h  */
/* ----------------------------------------------------------------------------------------------- */
