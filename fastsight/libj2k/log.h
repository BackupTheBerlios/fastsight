/**
 * log.h - generic logging functions
 *
 * Copyright (c) 2004 Daniel Atallah
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS `AS IS'
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

/**
 * logging levels.
 */
typedef enum
{
	LOG_FATAL,   /**< Fatal errors.                  */
	LOG_ERROR,   /**< Errors.                        */
	LOG_WARNING, /**< Warnings.                      */
	LOG_DEBUG,   /**< General debugging Information. */
	LOG_TRACE    /**< General chatter.               */

} LogLevel;


/**
 * Prints the specified text to the the log
 * @param message	the message to be appended to the log
 * @param log_level	the log level at which the message should be printed (if current log level >= log_level
 */
void log_print(LogLevel log_level, const char *message, ...);

/**
 * Sets the log level at which subsequent messages should be logged
 * @param log_level	the new log level
 */
void set_log_level(LogLevel log_level);

/**
 * Check if the current log level allows logging at the specified level
 * @param  log_level	the log level to check for logability
 * @return int		1 if log_level will be logged, 0 if it will not
 */
int is_loggable(LogLevel log_level);
