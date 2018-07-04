/* ----------------------------------------------------------------------------------------------- */
/*  Copyright (c) 2014 - 2018 by Axel Kenzo, axelkenzo@mail.ru                                     */
/*                                                                                                 */
/*  Разрешается повторное распространение и использование как в виде исходного кода, так и         */
/*  в двоичной форме, с изменениями или без, при соблюдении следующих условий:                     */
/*                                                                                                 */
/*   1. При повторном распространении исходного кода должно оставаться указанное выше уведомление  */
/*      об авторском праве, этот список условий и последующий отказ от гарантий.                   */
/*   2. При повторном распространении двоичного кода должна сохраняться указанная выше информация  */
/*      об авторском праве, этот список условий и последующий отказ от гарантий в документации     */
/*      и/или в других материалах, поставляемых при распространении.                               */
/*   3. Ни имя владельца авторских прав, ни имена его соратников не могут быть использованы в      */
/*      качестве рекламы или средства продвижения продуктов, основанных на этом ПО без             */
/*      предварительного письменного разрешения.                                                   */
/*                                                                                                 */
/*  ЭТА ПРОГРАММА ПРЕДОСТАВЛЕНА ВЛАДЕЛЬЦАМИ АВТОРСКИХ ПРАВ И/ИЛИ ДРУГИМИ СТОРОНАМИ "КАК ОНА ЕСТЬ"  */
/*  БЕЗ КАКОГО-ЛИБО ВИДА ГАРАНТИЙ, ВЫРАЖЕННЫХ ЯВНО ИЛИ ПОДРАЗУМЕВАЕМЫХ, ВКЛЮЧАЯ, НО НЕ             */
/*  ОГРАНИЧИВАЯСЬ ИМИ, ПОДРАЗУМЕВАЕМЫЕ ГАРАНТИИ КОММЕРЧЕСКОЙ ЦЕННОСТИ И ПРИГОДНОСТИ ДЛЯ КОНКРЕТНОЙ */
/*  ЦЕЛИ. НИ В КОЕМ СЛУЧАЕ НИ ОДИН ВЛАДЕЛЕЦ АВТОРСКИХ ПРАВ И НИ ОДНО ДРУГОЕ ЛИЦО, КОТОРОЕ МОЖЕТ    */
/*  ИЗМЕНЯТЬ И/ИЛИ ПОВТОРНО РАСПРОСТРАНЯТЬ ПРОГРАММУ, КАК БЫЛО СКАЗАНО ВЫШЕ, НЕ НЕСЁТ              */
/*  ОТВЕТСТВЕННОСТИ, ВКЛЮЧАЯ ЛЮБЫЕ ОБЩИЕ, СЛУЧАЙНЫЕ, СПЕЦИАЛЬНЫЕ ИЛИ ПОСЛЕДОВАВШИЕ УБЫТКИ,         */
/*  ВСЛЕДСТВИЕ ИСПОЛЬЗОВАНИЯ ИЛИ НЕВОЗМОЖНОСТИ ИСПОЛЬЗОВАНИЯ ПРОГРАММЫ (ВКЛЮЧАЯ, НО НЕ             */
/*  ОГРАНИЧИВАЯСЬ ПОТЕРЕЙ ДАННЫХ, ИЛИ ДАННЫМИ, СТАВШИМИ НЕПРАВИЛЬНЫМИ, ИЛИ ПОТЕРЯМИ ПРИНЕСЕННЫМИ   */
/*  ИЗ-ЗА ВАС ИЛИ ТРЕТЬИХ ЛИЦ, ИЛИ ОТКАЗОМ ПРОГРАММЫ РАБОТАТЬ СОВМЕСТНО С ДРУГИМИ ПРОГРАММАМИ),    */
/*  ДАЖЕ ЕСЛИ ТАКОЙ ВЛАДЕЛЕЦ ИЛИ ДРУГОЕ ЛИЦО БЫЛИ ИЗВЕЩЕНЫ О ВОЗМОЖНОСТИ ТАКИХ УБЫТКОВ.            */
/*                                                                                                 */
/*   ak_bckey.c                                                                                    */
/* ----------------------------------------------------------------------------------------------- */
 #include <ak_bckey.h>

/* ----------------------------------------------------------------------------------------------- */
/*! Функция устанавливает параметры алгоритма блочного шифрования, передаваемые в качестве
    аргументов. После инициализации остаются неопределенными следующие поля и методы,
    зависящие от конкретной реализации алгоритма блочного шифрования:

    - bkey.encrypt -- алгоритм зашифрования одного блока
    - bkey.decrypt -- алгоритм расшифрования одного блока
    - bkey.shedule_keys -- алгоритм развертки ключа и генерации раундовых ключей
    - bkey.delete_keys -- функция удаления раундовых ключей
    - bkey.key.data -- указатель на служебную область памяти
    - bkey.key.resource.counter -- максимально возможное число обрабатываемых блоков информации
    - bkey.key.oid -- идентификатор алгоритма шифрования
    - bkey.key.set_mask -- функция установки маски ключа
    - bkey.key.remask -- функция выработки и установки новой маски ключа
    - bkey.key.set_icode -- функция вычисления кода целостности
    - bkey.key.check_icode -- функция проверки кода целостности

    Перечисленные методы должны определяться в производящих функциях,
    создающих объекты конкретных алгоритмов блочного шифрования.

    @param bkey контекст ключа алгоритма блочного шифрованния
    @param keysize длина ключа в байтах
    @param blocksize длина блока обрабатываемых данных в байтах
    @return В случае успеха функция возвращает \ref ak_error_ok (ноль).
    В противном случае, возвращается код ошибки.                                                   */
/* ----------------------------------------------------------------------------------------------- */
 int ak_bckey_create( ak_bckey bkey, size_t keysize, size_t blocksize )
{
  int error = ak_error_ok;
  if( bkey == NULL ) return ak_error_message( ak_error_null_pointer, __func__,
                                                   "using a null pointer to block cipher context" );
  if( !keysize ) return ak_error_message( ak_error_zero_length, __func__,
                                                        "using block cipher key with zero length" );
  if( !blocksize ) return ak_error_message( ak_error_zero_length, __func__,
                                                            "using cipher with zero block length" );
 /* теперь инициализируем данные,
    для ключей блочного шифрования длина контрольной суммы всегда равна 8 байт */
  if(( error = ak_skey_create( &bkey->key, keysize, 8 )) != ak_error_ok )
    return ak_error_message( error, __func__, "wrong creation of secret key" );

 /* длина инициализационного вектора всегда совпадает с длиной блока данных */
  if(( error = ak_buffer_create_size( &bkey->ivector, blocksize )) != ak_error_ok ) {
    if( ak_skey_destroy( &bkey->key ) != ak_error_ok )
      ak_error_message( ak_error_get_value(), __func__, "wrong destroying a secret key" );
    return ak_error_message( error, __func__, "wrong memory allocation for temporary vector");
  }
  bkey->encrypt =       NULL;
  bkey->decrypt =       NULL;
  bkey->schedule_keys = NULL;
  bkey->delete_keys =   NULL;

 return ak_error_ok;
}

/* ----------------------------------------------------------------------------------------------- */
/*! @param bkey контекст ключа алгоритма блочного шифрованния
    @return В случае успеха функция возввращает \ref ak_error_ok (ноль).
    В противном случае, возвращается код ошибки.                                                   */
/* ----------------------------------------------------------------------------------------------- */
 int ak_bckey_destroy( ak_bckey bkey )
{
  int error = ak_error_ok;
  if( bkey == NULL ) return ak_error_message( ak_error_null_pointer, __func__,
                                                  "using a null pointer to block cipher context" );
  if( bkey->delete_keys != NULL ) {
    if(( error = bkey->delete_keys( &bkey->key )) != ak_error_ok ) {
      ak_error_message( error, __func__ , "wrong deleting of round keys" );
    }
  }
  if(( error = ak_buffer_wipe( &bkey->ivector, &bkey->key.generator )) != ak_error_ok )
    ak_error_message( error, __func__, "wrong wiping a temporary vector");
  if(( error = ak_buffer_destroy( &bkey->ivector )) != ak_error_ok )
    ak_error_message( error, __func__, "wrong destroying a temporary vector" );
  if(( error = ak_skey_destroy( &bkey->key )) != ak_error_ok )
    ak_error_message( error, __func__, "wrong destroying a secret key" );

  bkey->encrypt =       NULL;
  bkey->decrypt =       NULL;
  bkey->schedule_keys = NULL;
  bkey->delete_keys =   NULL;

 return error;
}

/* ----------------------------------------------------------------------------------------------- */
/*! @param bkey контекст ключа алгоритма блочного шифрованния
    @return Функция всегда возвращает NULL.                                                        */
/* ----------------------------------------------------------------------------------------------- */
 ak_pointer ak_bckey_delete( ak_pointer bkey )
{
  if( bkey != NULL ) {
    ak_bckey_destroy( bkey );
    free( bkey );
  } else ak_error_message( ak_error_null_pointer, __func__ ,
                                                         "using null pointer to block cipher key" );
 return NULL;
}


/* ----------------------------------------------------------------------------------------------- */
/*! Функция присваивает контексту ключа алгоритма блочного шифрования заданное значение,
    содержащееся в области памяти, на которую указывает аргумент функции ptr.
    При инициализации значение ключа \b копируется в контекст ключа, если флаг cflag истиннен.
    Если флаг ложен, то копирования (размножения ключевой информации) не происходит.
    Поведение функции при копировании аналогично поведению функции ak_buffer_set_ptr().

    Перед присвоением ключа контекст должен быть инициализирован.
    После присвоения ключа производится его маскирование и выработка контрольной суммы.

    Предпалагается, что основное использование функции ak_bckey_context_set_ptr()
    заключается в тестировании алгоритма блочного шифрования на заданных (тестовых)
    значениях ключей.

    @param bkey Контекст ключа блочного алгоритма шифрования.
    @param keyptr Указатель на область памяти, содержащую значение ключа.
    @param size Размер области памяти, содержащей значение ключа.
    @param cflag Флаг владения данными. Если он истинен, то данные копируются в область памяти,
    которой владеет ключевой контекст.

    @return Функция возвращает код ошибки. В случае успеха возвращается \ref ak_error_ok.          */
/* ----------------------------------------------------------------------------------------------- */
 int ak_bckey_context_set_ptr( ak_bckey bkey,
                                   const ak_pointer keyptr, const size_t size, const ak_bool cflag )
{
  int error = ak_error_ok;

 /* проверяем входные данные */
  if( bkey == NULL ) return ak_error_message( ak_error_null_pointer, __func__,
                                                        "using null pointer to secret key context" );
  if( keyptr == NULL ) return ak_error_message( ak_error_null_pointer, __func__ ,
                                                                  "using null pointer to key data" );
  if( size != bkey->key.key.size ) return ak_error_message( ak_error_wrong_length, __func__,
                                         "using a constant value for secret key with wrong length" );
 /* присваиваем ключевой буффер */
  if(( error = ak_skey_set_ptr( &bkey->key, keyptr, size, cflag )) != ak_error_ok )
    return ak_error_message( error, __func__ , "incorrect assigning of key data" );

 /* выполняем развертку раундовых ключей */
  if( bkey->schedule_keys != NULL ) bkey->schedule_keys( &bkey->key );

 return error;
}

/* ----------------------------------------------------------------------------------------------- */
/*! Функция присваивает контексту ключа алгоритма блочного шифрования случайное
    или псевдо-случайным значение, вырабатываемой заданным генератором. Размер вырабатываемого
    значения определяется длиной ключа.

    Перед присвоением ключа контекст должен быть инициализирован.
    После присвоения ключа производится его маскирование и выработка контрольной суммы.

    @param bkey Контекст ключа блочного алгоритма шифрования.
    @param generator Rонтекст генератора псевдо-случайных чисел.

    @return В случае успеха возвращается значение \ref ak_error_ok. В противном случае
    возвращается код ошибки.                                                                       */
/* ----------------------------------------------------------------------------------------------- */
 int ak_bckey_context_set_random( ak_bckey bkey, ak_random generator )
{
  int error = ak_error_ok;

 /* проверяем входные данные */
  if( bkey == NULL ) return ak_error_message( ak_error_null_pointer, __func__,
                                                      "using null pointer to secret key context" );
  if( generator == NULL ) return ak_error_message( ak_error_null_pointer, __func__,
                                                 "using null pointer to random number generator" );
 /* вырабатываем ключевой буффер */
  if(( error = ak_skey_set_random( &bkey->key, generator )) != ak_error_ok )
    return ak_error_message( error, __func__ , "incorrect generation of secret key random value" );

 /* выполняем развертку раундовых ключей */
  if( bkey->schedule_keys != NULL ) bkey->schedule_keys( &bkey->key );

 return error;
}

/* ----------------------------------------------------------------------------------------------- */
/*! Функция присваивает ключу значение, выработанное из заданного пароля при помощи алгоритма,
    описанного  в рекомендациях по стандартизации Р 50.1.111-2016.

    Пароль является секретным значением и должен быть не пустой строкой символов в формате utf8.
    Используемое при выработке ключа значение инициализационного вектора может быть не секретным.
    Перед присвоением ключа контекст должен быть инициализирован.

    @param bkey Контекст ключа блочного алгоритма шифрования.
    @param pass Пароль, представленный в виде строки символов в формате utf8.
    @param pass_size Длина пароля в байтах
    @param salt Инициализационный вектор, представленный в виде строки символов.
    @param salt_size Длина инициализационного вектора в байтах

    @return В случае успеха возвращается значение \ref ak_error_ok. В противном случае
    возвращается код ошибки.                                                                       */
/* ----------------------------------------------------------------------------------------------- */
 int ak_bckey_context_set_password( ak_bckey bkey, const ak_pointer pass, const size_t pass_size,
                                                     const ak_pointer salt, const size_t salt_size )
{
  int error = ak_error_ok;

 /* проверяем входные данные */
  if( bkey == NULL ) return ak_error_message( ak_error_null_pointer, __func__,
                                                  "using null pointer to block cipher key context" );
  if( pass == NULL ) return ak_error_message( ak_error_null_pointer, __func__ ,
                                                                  "using null pointer to password" );
  if( pass_size == 0 ) return ak_error_message( ak_error_zero_length, __func__ ,
                                                                 "using password with zero length" );
  if( salt == NULL ) return ak_error_message( ak_error_null_pointer, __func__ ,
                                                            "using null pointer to initial vector" );
  if( salt_size == 0 ) return ak_error_message( ak_error_zero_length, __func__ ,
                                                           "using initial vector with zero length" );
 /* вырабатываем ключевой буффер */
  if(( error = ak_skey_set_password( &bkey->key, pass, pass_size, salt, salt_size )) != ak_error_ok )
    return ak_error_message( error, __func__ , "incorrect generation of secret key random value" );

 /* выполняем развертку раундовых ключей */
  if( bkey->schedule_keys != NULL ) bkey->schedule_keys( &bkey->key );

 return error;
}

/* ----------------------------------------------------------------------------------------------- */
/*                                        теперь режимы шифрования                                 */
/* ----------------------------------------------------------------------------------------------- */
/*! @param bkey Ключ алгоритма блочного шифрования, на котором происходит зашифрование информации
    @param in Указатель на область памяти, где хранятся входные (зашифровываемые) данные
    @param out Указатель на область памяти, куда помещаются зашифрованные данные
    (этот указатель может совпадать с in)
    @param size Размер зашировываемых данных (в байтах). Для режима простой замены
    длина зашифровываемых данных должна быть кратна длине блока.

    @return В случае возникновения ошибки функция возвращает ее код, в противном случае
    возвращается ak_error_ok (ноль)                                                                */
/* ----------------------------------------------------------------------------------------------- */
 int ak_bckey_context_encrypt_ecb( ak_bckey bkey, ak_pointer in, ak_pointer out, size_t size )
{
  ak_int64 blocks = 0;
  ak_uint64 *inptr = (ak_uint64 *)in, *outptr = (ak_uint64 *)out;

 /* выполняем проверку размера входных данных */
  if( size%bkey->ivector.size != 0 )
    return ak_error_message( ak_error_wrong_block_cipher_length,
                            __func__ , "the length of input data is not divided by block length" );

 /* проверяем целостность ключа */
  if( bkey->key.check_icode( &bkey->key ) != ak_true )
    return ak_error_message( ak_error_wrong_key_icode,
                                        __func__, "incorrect integrity code of secret key value" );
 /* уменьшаем значение ресурса ключа */
  blocks = (ak_uint64 ) size/bkey->ivector.size;
  if( bkey->key.resource.counter < blocks )
    return ak_error_message( ak_error_low_key_resource,
                                                   __func__ , "low resource of block cipher key" );
   else bkey->key.resource.counter -= blocks;

 /* теперь приступаем к зашифрованию данных */
  if( bkey->ivector.size == 8 ) { /* здесь длина блока равна 64 бита */
    do {
        bkey->encrypt( &bkey->key, inptr++, outptr++ );
    } while( --blocks > 0 );
  }
  if( bkey->ivector.size == 16 ) { /* здесь длина блока равна 128 бит */
    do {
        bkey->encrypt( &bkey->key, inptr, outptr );
        inptr+=2; outptr+=2;
    } while( --blocks > 0 );
  }

  /* перемаскируем ключ */
  if( bkey->key.remask( &bkey->key ) != ak_error_ok )
    ak_error_message( ak_error_get_value(), __func__ , "wrong remasking of secret key" );

 return ak_error_ok;
}

/* ----------------------------------------------------------------------------------------------- */
/*! @param bkey Ключ алгоритма блочного шифрования, на котором происходит расшифрование информации
    @param in Указатель на область памяти, где хранятся входные (расшифровываемые) данные
    @param out Указатель на область памяти, куда помещаются расшифрованные данные
    (этот указатель может совпадать с in)
    @param size Размер расшировываемых данных (в байтах). Для режима простой замены
    длина расшифровываемых данных должна быть кратна длине блока.

    @return В случае возникновения ошибки функция возвращает ее код, в противном случае
    возвращается ak_error_ok (ноль)                                                                */
/* ----------------------------------------------------------------------------------------------- */
 int ak_bckey_context_decrypt_ecb( ak_bckey bkey, ak_pointer in, ak_pointer out, size_t size )
{
  ak_int64 blocks = 0;
  ak_uint64 *inptr = (ak_uint64 *)in, *outptr = (ak_uint64 *)out;

 /* выполняем проверку размера входных данных */
  if( size%bkey->ivector.size != 0 )
    return ak_error_message( ak_error_wrong_block_cipher_length,
                            __func__ , "the length of input data is not divided by block length" );

 /* проверяем целостность ключа */
  if( bkey->key.check_icode( &bkey->key ) != ak_true )
    return ak_error_message( ak_error_wrong_key_icode,
                                        __func__, "incorrect integrity code of secret key value" );
 /* уменьшаем значение ресурса ключа */
  blocks = (ak_uint64 ) size/bkey->ivector.size;
  if( bkey->key.resource.counter < blocks )
    return ak_error_message( ak_error_low_key_resource,
                                                   __func__ , "low resource of block cipher key" );
   else bkey->key.resource.counter -= blocks;

 /* теперь приступаем к расшифрованию данных */
  if( bkey->ivector.size == 8 ) { /* здесь длина блока равна 64 бита */
    do {
        bkey->decrypt( &bkey->key, inptr++, outptr++ );
    } while( --blocks > 0 );
  }
  if( bkey->ivector.size == 16 ) { /* здесь длина блока равна 128 бит */
    do {
        bkey->decrypt( &bkey->key, inptr, outptr );
        inptr+=2; outptr+=2;
    } while( --blocks > 0 );
  }

  /* перемаскируем ключ */
  if( bkey->key.remask( &bkey->key ) != ak_error_ok )
    ak_error_message( ak_error_get_value(), __func__ , "wrong remasking of secret key" );

 return ak_error_ok;
}

/* ----------------------------------------------------------------------------------------------- */
/*! Поскольку операцией заширования является гаммирование (сложение открытого текста по модулю два
    с последовательностью, вырабатываемой шифром), то операция расшифрования производится также
    наложением гаммы по модулю два. Таким образом, для зашифрования и расшифрования
    информациии используется одна и таже функция.

    @param bkey Ключ алгоритма блочного шифрования, на котором происходит зашифрование/расшифрование информации.
    @param in Указатель на область памяти, где хранятся входные (открытые) данные.
    @param out Указатель на область памяти, куда помещаются зашифрованные данные
    (этот указатель может совпадать с in).
    @param size Размер зашировываемых данных (в байтах).
    @param iv Синхропосылка. Согласно  стандарту ГОСТ Р 34.13-2015 длина синхропосылки должна быть
    ровно в два раза меньше, чем длина блока, то есть 4 байта для Магмы и 8 байт для Кузнечика.
    @param iv_size Длина синхропосылки (в байтах).

    Значение синхропосылки преобразуется и сохраняется в контексте секретного ключа. Данное значение
    может быть использовано в дальнейшем при вызове функции ak_bckey_xcrypt_update().

    @return В случае возникновения ошибки функция возвращает ее код, в противном случае
    возвращается ak_error_ok (ноль)                                                                */
/* ----------------------------------------------------------------------------------------------- */
 int ak_bckey_context_xcrypt( ak_bckey bkey, ak_pointer in, ak_pointer out, size_t size,
                                                                     ak_pointer iv, size_t iv_size )
{
  ak_int64 blocks = (ak_int64)size/bkey->ivector.size,
            tail = (ak_int64)size%bkey->ivector.size;
  ak_uint64 yaout[2], *inptr = (ak_uint64 *)in, *outptr = (ak_uint64 *)out;

 /* проверяем целостность ключа */
  if( bkey->key.check_icode( &bkey->key ) != ak_true )
    return ak_error_message( ak_error_wrong_key_icode, __func__,
                                                   "incorrect integrity code of secret key value" );
 /* проверяем длину синхропосылки (если меньше половины блока, то плохо)
    если больше - то лишнее не используется */
  if( iv_size < ( bkey->ivector.size >> 1 ))
    return ak_error_message( ak_error_wrong_iv_length, __func__,
                                                              "incorrect length of initial value" );
 /* уменьшаем значение ресурса ключа */
  if( bkey->key.resource.counter < ( blocks + ( tail > 0 )))
    return ak_error_message( ak_error_low_key_resource,
                                                    __func__ , "low resource of block cipher key" );
   else bkey->key.resource.counter -= ( blocks + ( tail > 0 )); /* уменьшаем ресурс ключа */

 /* теперь приступаем к зашифрованию данных */
  if( bkey->key.flags&ak_flag_xcrypt_update ) bkey->key.flags ^= ak_flag_xcrypt_update;
  memset( bkey->ivector.data, 0, bkey->ivector.size );

  if( blocks ) {
   /* здесь длина блока равна 64 бита */
    if( bkey->ivector.size == 8 ) {
      memcpy( ((ak_uint8 *)bkey->ivector.data)+4, iv, 4 );
      do {
          bkey->encrypt( &bkey->key, bkey->ivector.data, yaout );
          *outptr = *inptr ^ yaout[0];
          outptr++; inptr++; ((ak_uint64 *)bkey->ivector.data)[0]++;
      } while( --blocks > 0 );
    }

   /* здесь длина блока равна 128 бит */
    if( bkey->ivector.size == 16 ) {
      memcpy( ((ak_uint8 *)bkey->ivector.data)+8, iv, 8 );
      do {
          bkey->encrypt( &bkey->key, bkey->ivector.data, yaout );
          *outptr = *inptr ^ yaout[0]; outptr++; inptr++;
          *outptr = *inptr ^ yaout[1]; outptr++; inptr++;
          ((ak_uint64 *)bkey->ivector.data)[0]++; // здесь мы не учитываем знак переноса
                                                  // потому что объем данных на одном ключе не должен превышать
                                                  // 2^64 блоков (контролируется через ресурс ключа)
      } while( --blocks > 0 );
    }
  }

  if( tail ) { /* на последок, мы обрабатываем хвост сообщения */
    size_t i;
    bkey->encrypt( &bkey->key, bkey->ivector.data, yaout );
    for( i = 0; i < tail; i++ )
        ( (ak_uint8*)outptr )[i] = ( (ak_uint8*)inptr )[i]^( (ak_uint8 *)yaout)[i];
   /* запрещаем дальнейшее использование xcrypt_update для данных, длина которых не кратна длине блока */
    memset( bkey->ivector.data, 0, bkey->ivector.size );
    bkey->key.flags |= ak_flag_xcrypt_update;
  }

 /* перемаскируем ключ */
  if( bkey->key.remask( &bkey->key ) != ak_error_ok )
    return ak_error_message( ak_error_get_value(), __func__ , "wrong remasking of secret key" );

 return ak_error_ok;
}

/* ----------------------------------------------------------------------------------------------- */
/*! Функция позволяет зашифровывать/расшифровывать данные после вызова функции ak_bckey_xcrypt()
    со значением синхропосылки, выработанной в ходе предыдущего вызова. Это позволяет
    зашифровывать/расшифровывать данные поступающие блоками, длина которых кратна длине блока
    используемого алгоритма блочного шифрования.

    @param bkey Ключ алгоритма блочного шифрования, на котором происходит зашифрование информации
    @param in Указатель на область памяти, где хранятся входные (открытые) данные
    @param out Указатель на область памяти, куда помещаются зашифрованные данные
    (этот указатель может совпадать с in)
    @param size Размер зашировываемых данных (в байтах)

    @return В случае возникновения ошибки функция возвращает ее код, в противном случае
    возвращается ak_error_ok (ноль)                                                                */
/* ----------------------------------------------------------------------------------------------- */
 int ak_bckey_context_xcrypt_update( ak_bckey bkey, ak_pointer in, ak_pointer out, size_t size )
{
  ak_int64 blocks = (ak_int64)size/bkey->ivector.size,
            tail = (ak_int64)size%bkey->ivector.size;
  ak_uint64 yaout[2], *inptr = (ak_uint64 *)in, *outptr = (ak_uint64 *)out;

 /* проверяем, что мы можем использовать данный режим */
  if( bkey->key.flags&ak_flag_xcrypt_update )
    return ak_error_message( ak_error_wrong_block_cipher_function, __func__ ,
                                  "using this function with previously incorrect xcrypt operation");
 /* проверяем целостность ключа */
  if( bkey->key.check_icode( &bkey->key ) != ak_true )
    return ak_error_message( ak_error_wrong_key_icode, __func__,
                                                   "incorrect integrity code of secret key value" );
 /* уменьшаем значение ресурса ключа */
  if( bkey->key.resource.counter < ( blocks + ( tail > 0 )))
    return ak_error_message( ak_error_low_key_resource,
                                                    __func__ , "low resource of block cipher key" );
   else bkey->key.resource.counter -= ( blocks + ( tail > 0 )); /* уменьшаем ресурс ключа */

 /* теперь приступаем к зашифрованию данных */
  if( blocks ) {
    if( bkey->ivector.size == 8 ) { /* здесь длина блока равна 64 бита */
      do {
          bkey->encrypt( &bkey->key, bkey->ivector.data, yaout );
          *outptr = *inptr ^ yaout[0];
          outptr++; inptr++; ((ak_uint64 *)bkey->ivector.data)[0]++;
      } while( --blocks > 0 );
    }

    if( bkey->ivector.size == 16 ) { /* здесь длина блока равна 128 бит */
      do {
          bkey->encrypt( &bkey->key, bkey->ivector.data, yaout );
          *outptr = *inptr ^ yaout[0]; outptr++; inptr++;
          *outptr = *inptr ^ yaout[1]; outptr++; inptr++;
          ((ak_uint64 *)bkey->ivector.data)[0]++;
        } while( --blocks > 0 );
    }
  }

  if( tail ) { /* на последок, мы обрабатываем хвост сообщения */
    size_t i;
    bkey->encrypt( &bkey->key, bkey->ivector.data, yaout );
    for( i = 0; i < tail; i++ )
        ( (ak_uint8*)outptr )[i] = ( (ak_uint8*)inptr )[i]^( (ak_uint8 *)yaout)[i];
    memset( bkey->ivector.data, 0, bkey->ivector.size );
    bkey->key.flags |= ak_flag_xcrypt_update;
  }

 /* перемаскируем ключ */
  if( bkey->key.remask( &bkey->key ) != ak_error_ok )
    return ak_error_message( ak_error_get_value(), __func__ , "wrong remasking of secret key" );

 return ak_error_ok;
}


/* ----------------------------------------------------------------------------------------------- */
/*! Функция позволяет вычислить имитовставку от заданной лобласти памяти фиксированного размера.

   @param bkey Ключ алгоритма блочного шифрования, используемый для выработки имитовставки.
   Ключ должен быть создан и определен.
   @param in Указатель на входные данные для которых вычисляется имитовставка.
   @param size Размер входных данных в байтах.
   @param out Область памяти, куда будет помещен результат. Память должна быть заранее выделена.
   Размер выделяемой памяти должен совпадать с длиной блока используемого алгоритма
   блочного шифрования. Указатель out может принимать значение NULL.

   @return Функция возвращает NULL, если указатель out не есть NULL, в противном случае
   возвращается указатель на буффер, содержащий результат вычислений. В случае возникновения
   ошибки возвращается NULL, при этом код ошибки может быть получен с помощью вызова функции
   ak_error_get_value().                                                                          */
/* ----------------------------------------------------------------------------------------------- */
 ak_buffer ak_bckey_context_mac_gost3413( ak_bckey bkey, ak_pointer in, size_t size, ak_pointer out )
{
  ak_pointer pout = NULL;
  ak_buffer result = NULL;
  ak_int64 i = 0,
           blocks = (ak_int64)size/bkey->ivector.size,
           tail = (ak_int64)size%bkey->ivector.size;
  ak_uint64 yaout[2], akey[2], *inptr = (ak_uint64 *)in;

 /* проверяем, что длина данных больше нуля */
  if( !size ) {
    ak_error_message( ak_error_zero_length, __func__, "using a data with zero length" );
    return NULL;
  }

 /* проверяем целостность ключа */
  if( bkey->key.check_icode( &bkey->key ) != ak_true ) {
    ak_error_message( ak_error_wrong_key_icode, __func__,
                                                  "incorrect integrity code of secret key value" );
    return NULL;
  }

 /* уменьшаем значение ресурса ключа */
  if( bkey->key.resource.counter < ( blocks + ( tail > 0 ))) {
    ak_error_message( ak_error_low_key_resource, __func__ , "low resource of block cipher key" );
    return NULL;
  } else bkey->key.resource.counter -= ( blocks + ( tail > 0 )); /* уменьшаем ресурс ключа */

  memset( akey, 0, sizeof( akey ));
  memset( yaout, 0, sizeof( yaout ));
  if( !tail ) { tail = bkey->ivector.size; blocks--; } /* последний блок всегда существует */

 /* основной цикл */
  if( bkey->ivector.size == 16 ) { /* здесь длина блока равна 128 бита */
    for( i = 0; i < blocks; i++, inptr += 2 ) {
       yaout[0] ^= inptr[0];
       yaout[1] ^= inptr[1];
       bkey->encrypt( &bkey->key, yaout, yaout );
    }

    bkey->encrypt( &bkey->key, akey, akey );
    if( akey[1]&0x8000000000000000LL ) {
      akey[1] <<= 1; akey[1] ^= (akey[0] >> 63); akey[0] <<= 1;
      akey[0] ^= 0x87; //135
    } else {
        akey[1] <<= 1; akey[1] ^= (akey[0] >> 63); akey[0] <<= 1;
      }

    if( tail < bkey->ivector.size ) {
        if( akey[1]&0x8000000000000000LL ) {
          akey[1] <<= 1; akey[1] ^= (akey[0] >> 63); akey[0] <<= 1;
          akey[0] ^= 0x87; //135
        } else {
            akey[1] <<= 1; akey[1] ^= (akey[0] >> 63); akey[0] <<= 1;
          }
    }

    // теперь шифруем последний блок
    akey[0] ^= yaout[0]; akey[1] ^= yaout[1];
    for( i = 0; i < tail; i++ ) ((ak_uint8 *)akey)[i] ^= ((ak_uint8 *)inptr)[i];
    bkey->encrypt( &bkey->key, akey, akey );
  }

  if( bkey->ivector.size == 8 ) { /* здесь длина блока равна 64 бита */
    for( i = 0; i < blocks; i++, inptr++ ) {
       yaout[0] ^= inptr[0];
       bkey->encrypt( &bkey->key, yaout, yaout );
    }

    bkey->encrypt( &bkey->key, akey, akey );
    if( akey[0]&0x8000000000000000LL ) {
      akey[0] <<= 1; akey[0] ^= 0x1B;
    } else akey[0] <<= 1;

    if( tail < bkey->ivector.size ) {
      if( akey[0]&0x8000000000000000LL ) {
        akey[0] <<= 1; akey[0] ^= 0x1B;
      } else akey[0] <<= 1;
    }

    // теперь шифруем последний блок
    akey[0] ^= yaout[0];
    for( i = 0; i < tail; i++ ) ((ak_uint8 *)akey)[i] ^= ((ak_uint8 *)inptr)[i];
    bkey->encrypt( &bkey->key, akey, akey );
  }

 /* определяем указатель на область памяти, в которую будет помещен результат вычислений */
  if( out != NULL ) pout = out;
   else {
     if(( result = ak_buffer_new_size( bkey->ivector.size )) != NULL ) pout = result->data;
      else ak_error_message( ak_error_get_value( ), __func__ , "wrong creation of result buffer" );
   }
 /* копируем нужную часть результирующего массива или выдаем сообщение об ошибке */
  if( pout != NULL ) memcpy( pout, akey, bkey->ivector.size );
    else ak_error_message( ak_error_out_of_memory, __func__ ,
                                                 "incorrect memory allocation for result buffer" );
 return result;
}

/* ----------------------------------------------------------------------------------------------- */
/*! \example example-bckey-internal.c                                                              */
/* ----------------------------------------------------------------------------------------------- */
/*                                                                                     ak_bckey.c  */
/* ----------------------------------------------------------------------------------------------- */
