#include <stdio.h>
#include <stdlib.h>
#include "c4_model.h"

int main(void)
{
    printf("=== C4 Model Example ===\n\n");

    C4Model model;
    c4_model_init(&model, "Online Banking Platform",
                  "A digital banking system for retail customers");

    c4_model_add_person(&model, "Customer",
                         "A bank customer using online banking",
                         false);
    c4_model_add_person(&model, "Bank Employee",
                         "Internal bank staff member",
                         false);
    c4_model_add_person(&model, "Regulator",
                         "Financial regulatory authority",
                         true);

    unsigned int banking_system = c4_model_add_system(&model,
        "Online Banking System",
        "Allows customers to view accounts, make payments, and manage "
        "their finances through a web and mobile interface.",
        "Bank Data Center", false);

    unsigned int mainframe = c4_model_add_system(&model,
        "Mainframe Banking System",
        "Stores all core banking customer accounts, transactions, and "
        "financial records. Legacy COBOL system.",
        "Core Banking", true);

    unsigned int email_system = c4_model_add_system(&model,
        "E-mail System",
        "Sends e-mail notifications to customers for transactions, "
        "statements, and alerts.",
        "Email Services", true);

    c4_model_add_relationship(&model,
        "Customer", "Online Banking System",
        C4_REL_USES, "Views account balances and makes payments", "HTTPS");

    c4_model_add_relationship(&model,
        "Bank Employee", "Online Banking System",
        C4_REL_USES, "Administers customer accounts", "HTTPS");

    c4_model_add_relationship(&model,
        "Online Banking System", "Mainframe Banking System",
        C4_REL_USES, "Retrieves and updates account data", "MQ/XML");

    c4_model_add_relationship(&model,
        "Online Banking System", "E-mail System",
        C4_REL_SENDS_TO, "Sends confirmation emails to customers", "SMTP");

    unsigned int web_app = c4_model_add_container(&model,
        "Web Application",
        "Delivers the static content and the single-page application for "
        "the online banking interface.",
        "React + Nginx", banking_system);

    unsigned int api_gateway = c4_model_add_container(&model,
        "API Gateway",
        "Routes and authenticates API requests from the SPA to backend "
        "services.",
        "Kong / Nginx", banking_system);

    unsigned int account_service = c4_model_add_container(&model,
        "Account Service",
        "Manages customer account operations: balance queries, transaction "
        "history, and account management.",
        "Java / Spring Boot", banking_system);

    unsigned int payment_service = c4_model_add_container(&model,
        "Payment Service",
        "Handles payment processing, transfers, and bill payments.",
        "Go / gRPC", banking_system);

    unsigned int notification_service = c4_model_add_container(&model,
        "Notification Service",
        "Manages email, SMS, and push notification delivery to customers.",
        "Node.js", banking_system);

    unsigned int customer_db = c4_model_add_container(&model,
        "Customer Database",
        "Stores customer profiles, credentials, and preferences.",
        "PostgreSQL 15", banking_system);

    unsigned int transaction_db = c4_model_add_container(&model,
        "Transaction Database",
        "Stores payment and transaction records with audit trail.",
        "PostgreSQL 15", banking_system);

    c4_model_add_relationship(&model,
        "Web Application", "API Gateway",
        C4_REL_CALLS, "Makes API calls for all operations", "HTTPS/REST");

    c4_model_add_relationship(&model,
        "API Gateway", "Account Service",
        C4_REL_CALLS, "Routes account-related requests", "gRPC");

    c4_model_add_relationship(&model,
        "API Gateway", "Payment Service",
        C4_REL_CALLS, "Routes payment-related requests", "gRPC");

    c4_model_add_relationship(&model,
        "API Gateway", "Notification Service",
        C4_REL_CALLS, "Routes notification requests", "gRPC");

    c4_model_add_relationship(&model,
        "Account Service", "Customer Database",
        C4_REL_STORED_IN, "Reads and writes customer data", "JDBC");

    c4_model_add_relationship(&model,
        "Payment Service", "Transaction Database",
        C4_REL_STORED_IN, "Stores transaction records", "JDBC");

    c4_model_add_relationship(&model,
        "Payment Service", "Mainframe Banking System",
        C4_REL_CALLS, "Executes financial transactions", "MQ/XML");

    c4_model_add_relationship(&model,
        "Notification Service", "E-mail System",
        C4_REL_SENDS_TO, "Sends notification emails", "SMTP");

    unsigned int auth_component = c4_model_add_component(&model,
        "Authentication Module",
        "Handles user login, MFA, and session management.",
        "Spring Security", account_service);

    unsigned int balance_component = c4_model_add_component(&model,
        "Balance Query Module",
        "Retrieves current account balances and pending transactions.",
        "Java", account_service);

    unsigned int transfer_component = c4_model_add_component(&model,
        "Transfer Processing Module",
        "Validates and executes fund transfers between accounts.",
        "Go", payment_service);

    c4_model_add_component(&model,
        "Fraud Detection Module",
        "Analyses transaction patterns to detect and flag potentially "
        "fraudulent activity.",
        "Python / ML", payment_service);

    c4_model_add_code_element(&model,
        "AuthController",
        "REST controller for login, logout, and token refresh endpoints.",
        "Java", auth_component);

    c4_model_add_code_element(&model,
        "TokenService",
        "Generates and validates JWT access and refresh tokens.",
        "Java", auth_component);

    c4_model_add_code_element(&model,
        "TransferValidator",
        "Validates transfer amount limits, KYC checks, and destination "
        "account format.",
        "Go", transfer_component);

    c4_model_link_adr_to_container(&model, customer_db, 1,
        "ADR #1: C99 selected as implementation language");
    c4_model_link_adr_to_container(&model, account_service, 2,
        "ADR #2: Layered architecture for separation of concerns");

    printf("\n");
    c4_model_print_context(&model);
    printf("\n");
    c4_model_print_container(&model);
    printf("\n");
    c4_model_print_component(&model, account_service);
    printf("\n");
    c4_model_print_code(&model, auth_component);
    printf("\n");

    printf("System count: %zu\n", model.system_count);
    printf("Container count: %zu\n", model.container_count);
    printf("Component count: %zu\n", model.component_count);
    printf("Code element count: %zu\n", model.code_element_count);
    printf("Relationship count: %zu\n", model.relationship_count);
    printf("Containers in Banking System: %zu\n",
           c4_model_get_container_count_for_system(&model, banking_system));
    printf("Components in Account Service: %zu\n",
           c4_model_get_component_count_for_container(
               &model, account_service));

    printf("\nExporting PlantUML context diagram...\n");
    c4_model_export_plantuml(&model, "context_diagram.puml",
                              C4_LEVEL_CONTEXT);
    printf("Exporting PlantUML container diagram...\n");
    c4_model_export_plantuml(&model, "container_diagram.puml",
                              C4_LEVEL_CONTAINER);

    if (c4_model_validate(&model)) {
        printf("C4 model validation: PASSED\n");
    }

    printf("\nDone.\n");
    return 0;
}
