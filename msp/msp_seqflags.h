/**
 * @file      msp_seqflags.h
 * @author    John Wikman
 * @copyright MIT License
 * @brief     Functionalities for keeping track of transaction-ID's in MSP.
 */

#ifndef MSP_SEQFLAGS_H
#define MSP_SEQFLAGS_H

/**
 * @brief A type for handling sequence flags in MSP.
 * 
 * The fields in the struct should never be accessed or modified directly. Use
 * the handler functions declared in msp_seqflags.h to interract with this
 * type.
 */
typedef struct {
	/**
	 * @brief Bit vector for keeping track of sequence flag values.
	 */
	unsigned short values[4];

	/**
	 * @brief Bit vector for checking if a sequence flag has been initialized or not.
	 */
	unsigned short inits[4];
} msp_seqflags_t;


msp_seqflags_t msp_seqflags_init(void);
int msp_seqflags_increment(volatile msp_seqflags_t *flags, unsigned char opcode);
int msp_seqflags_get(volatile const msp_seqflags_t *flags, unsigned char opcode);
int msp_seqflags_get_next(volatile const msp_seqflags_t *flags, unsigned char opcode);
int msp_seqflags_is_set(volatile const msp_seqflags_t *flags, unsigned char opcode, unsigned char flag);
int msp_seqflags_set(volatile msp_seqflags_t *flags, unsigned char opcode, unsigned char flag);

#endif /* MSP_SEQFLAGS_H */
