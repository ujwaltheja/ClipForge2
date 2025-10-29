package com.ucworks.clipforge.utils

import timber.log.Timber

/**
 * ErrorHandler - Centralized error handling and recovery
 * Manages error states, logging, and user feedback
 */
object ErrorHandler {

    /**
     * Error types for categorization
     */
    sealed class ErrorType {
        object NetworkError : ErrorType()
        object DatabaseError : ErrorType()
        object FileSystemError : ErrorType()
        object PermissionError : ErrorType()
        object InvalidDataError : ErrorType()
        object NativeError : ErrorType()
        object TimeoutError : ErrorType()
        object UnknownError : ErrorType()

        fun getDisplayMessage(): String = when (this) {
            NetworkError -> "Network error occurred"
            DatabaseError -> "Database error"
            FileSystemError -> "File system error"
            PermissionError -> "Permission denied"
            InvalidDataError -> "Invalid data"
            NativeError -> "Processing error"
            TimeoutError -> "Operation timed out"
            UnknownError -> "An unknown error occurred"
        }
    }

    /**
     * Handle exception and determine error type
     */
    fun handleException(exception: Exception): ErrorType {
        return when (exception) {
            is java.net.SocketException,
            is java.net.ConnectException,
            is java.net.UnknownHostException -> {
                Timber.e(exception, "Network error")
                ErrorType.NetworkError
            }
            is SecurityException -> {
                Timber.e(exception, "Permission error")
                ErrorType.PermissionError
            }
            is java.io.IOException -> {
                Timber.e(exception, "File system error")
                ErrorType.FileSystemError
            }
            is IllegalArgumentException,
            is IllegalStateException -> {
                Timber.e(exception, "Invalid data error")
                ErrorType.InvalidDataError
            }
            is java.util.concurrent.TimeoutException -> {
                Timber.e(exception, "Timeout error")
                ErrorType.TimeoutError
            }
            else -> {
                Timber.e(exception, "Unknown error")
                ErrorType.UnknownError
            }
        }
    }

    /**
     * Safe execution with error handling
     */
    inline fun <T> tryCatch(
        operation: String,
        block: () -> T,
        fallback: T? = null
    ): T? {
        return try {
            Timber.d("Starting operation: $operation")
            val result = block()
            Timber.d("Operation completed: $operation")
            result
        } catch (e: Exception) {
            Timber.e(e, "Operation failed: $operation")
            fallback
        }
    }

    /**
     * Safe execution with error callback
     */
    inline fun tryCatchWithCallback(
        operation: String,
        block: () -> Unit,
        onError: (Exception) -> Unit = {}
    ) {
        try {
            Timber.d("Starting operation: $operation")
            block()
            Timber.d("Operation completed: $operation")
        } catch (e: Exception) {
            Timber.e(e, "Operation failed: $operation")
            onError(e)
        }
    }

    /**
     * Get user-friendly error message
     */
    fun getErrorMessage(exception: Exception): String {
        return when {
            exception.message?.contains("permission", ignoreCase = true) == true -> {
                "Permission required to access this resource"
            }
            exception.message?.contains("network", ignoreCase = true) == true -> {
                "Network connection failed"
            }
            exception.message?.contains("timeout", ignoreCase = true) == true -> {
                "Operation took too long"
            }
            exception.message?.contains("not found", ignoreCase = true) == true -> {
                "Resource not found"
            }
            else -> exception.message ?: "An error occurred"
        }
    }

    /**
     * Log error details
     */
    fun logErrorDetails(
        operation: String,
        exception: Exception,
        metadata: Map<String, String> = emptyMap()
    ) {
        Timber.e("=== Error Details ===")
        Timber.e("Operation: $operation")
        Timber.e("Error Type: ${exception.javaClass.simpleName}")
        Timber.e("Message: ${exception.message}")
        Timber.e("Cause: ${exception.cause}")

        if (metadata.isNotEmpty()) {
            Timber.e("Metadata:")
            metadata.forEach { (key, value) ->
                Timber.e("  $key: $value")
            }
        }

        Timber.e("=== End Error ===")
        Timber.e(exception)
    }

    /**
     * Retry logic with exponential backoff
     */
    suspend fun <T> retryWithBackoff(
        maxRetries: Int = 3,
        initialDelayMs: Long = 100,
        operation: String,
        block: suspend () -> T
    ): T? {
        var lastException: Exception? = null
        var delayMs = initialDelayMs

        repeat(maxRetries) { attempt ->
            try {
                Timber.d("Attempt ${attempt + 1}/$maxRetries: $operation")
                return block()
            } catch (e: Exception) {
                lastException = e
                Timber.w("Attempt ${attempt + 1} failed, retrying in ${delayMs}ms")

                if (attempt < maxRetries - 1) {
                    kotlinx.coroutines.delay(delayMs)
                    delayMs *= 2  // Exponential backoff
                }
            }
        }

        Timber.e(lastException, "All retry attempts failed: $operation")
        return null
    }

    /**
     * Validate required field
     */
    fun validateRequired(
        fieldName: String,
        value: String?
    ): Boolean {
        if (value.isNullOrBlank()) {
            Timber.w("Required field is empty: $fieldName")
            return false
        }
        return true
    }

    /**
     * Validate field range
     */
    fun validateRange(
        fieldName: String,
        value: Int,
        min: Int,
        max: Int
    ): Boolean {
        if (value < min || value > max) {
            Timber.w("Field out of range: $fieldName ($value not in $min-$max)")
            return false
        }
        return true
    }

    /**
     * Validate field format
     */
    fun validateFormat(
        fieldName: String,
        value: String,
        pattern: Regex
    ): Boolean {
        if (!value.matches(pattern)) {
            Timber.w("Field format invalid: $fieldName")
            return false
        }
        return true
    }

    /**
     * Get stack trace as string
     */
    fun getStackTraceString(exception: Exception): String {
        val stackTrace = exception.stackTrace
            .map { "${it.className}.${it.methodName}(${it.fileName}:${it.lineNumber})" }
            .joinToString("\n  ")

        return """
            Exception: ${exception.javaClass.simpleName}
            Message: ${exception.message}
            Stack Trace:
              $stackTrace
        """.trimIndent()
    }
}

/**
 * Extension function for safe coroutine execution
 */
suspend inline fun <T> safeCall(
    operation: String,
    crossinline block: suspend () -> T,
    fallback: T? = null
): T? {
    return ErrorHandler.tryCatch(operation, {
        @Suppress("UNCHECKED_CAST")
        block() as T
    }, fallback)
}

/**
 * Extension function for Result type handling
 */
inline fun <T> runCatching(
    operation: String,
    block: () -> T
): Result<T> {
    return try {
        Timber.d("Starting: $operation")
        Result.success(block())
    } catch (e: Exception) {
        Timber.e(e, "Failed: $operation")
        Result.failure(e)
    }
}

/**
 * Extension function for Result handling
 */
fun <T> Result<T>.getOrLogError(operation: String = "Operation"): T? {
    return onSuccess {
        Timber.d("$operation succeeded")
    }.onFailure { exception ->
        ErrorHandler.logErrorDetails(operation, exception as Exception)
    }.getOrNull()
}
