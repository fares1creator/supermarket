#include "sm_header.h"

// ============================================================================
// MENU FUNCTIONS
// ============================================================================

void exportMenu(void) {
    int choice;
    
    do {
        printf("\n=== EXPORT DATA ===\n");
        printf("1. Export Products to CSV\n");
        printf("2. Export Sales to CSV\n");
        printf("3. Export Low Stock Report to CSV\n");
        printf("0. Back to Main Menu\n");
        printf("Enter choice: ");
        scanf("%d", &choice);
        
        switch (choice) {
            case 1: exportProductsToCSV(); break;
            case 2: exportSalesToCSV(); break;
            case 3: exportLowStockToCSV(); break;
            case 0: break;
            default: printf("Invalid choice!\n");
        }
    } while (choice != 0);
}

// ===== UPDATE YOUR EXISTING inventoryMenu() FUNCTION =====
void inventoryMenu() {
    int choice;
    
    while (1) {
        system("cls");
        printf("\n====================================\n");
        printf("|        INVENTORY MANAGEMENT      |\n");
        printf("====================================\n");
        printf("1. Add Product\n");
        printf("2. View All Products\n");
        printf("3. Update Product\n");
        printf("4. Delete Product\n");
        printf("5. Search Product by Name\n");
        printf("6. Search by Category\n");
        printf("7. 🔢 Sorting & View Options\n");         // Updated with icon
        printf("8. Set Product Discount\n");
        printf("9. View Promotions\n");
        printf("0. Back to Main Menu\n");
        printf("\n====================================\n");
        printf("Enter your choice: ");
        
        scanf("%d", &choice);
        getchar();
        
        switch (choice) {
            case 1: addProduct(); break;
            case 2: viewAllProducts(); break;
            case 3: updateProduct(); break;
            case 4: deleteProduct(); break;
            case 5: searchProductByName(); break;
            case 6: searchByCategory(); break;
            case 7: sortingMenu(); break;              // Call the new sorting menu
            case 8: setProductDiscount(); break;
            case 9: viewPromotions(); break;
            case 0: return;
            default: 
                printf("\n❌ Invalid choice! Please try again.\n");
                printf("Press any key to continue...");
                getchar();
        }
    }
}

void salesMenu(void) {
    int choice;
    
    do {
        printf("\n=== SALES MANAGEMENT ===\n");
        printf("1. Process New Sale\n");
        printf("2. View All Sales\n");
        printf("0. Back to Main Menu\n");
        printf("Enter choice: ");
        scanf("%d", &choice);
        
        switch (choice) {
            case 1: processNewSale(); break;
            case 2: viewAllSales(); break;
            case 0: break;
            default: printf("Invalid choice!\n");
        }
    } while (choice != 0);
}

void reportsMenu(void) {
    int choice;
    
    do {
        printf("\n=== REPORTS & ANALYTICS ===\n");
        printf("1. Low Stock Report\n");
        printf("2. Daily Sales Report\n");
        printf("3. Monthly Report\n");
        printf("4. Date Range Report\n");
        printf("5. Best Selling Products\n");
        printf("6. Category Analysis\n");
        printf("0. Back to Main Menu\n");
        printf("Enter choice: ");
        scanf("%d", &choice);
        
        switch (choice) {
            case 1: lowStockReport(); break;
            case 2: dailySalesReport(); break;
            case 3: monthlyReport(); break;
            case 4: dateRangeReport(); break;
            case 5: bestSellingProducts(); break;
            case 6: categoryReport(); break;
            case 0: break;
            default: printf("Invalid choice!\n");
        }
    } while (choice != 0);
}

void adminMenu(void) {
    if (!isAdmin) {
        printf("Access denied. Admin privileges required.\n");
        return;
    }
    
    int choice;
    
    do {
        printf("\n=== ADMIN PANEL ===\n");
        printf("1. Add User\n");
        printf("2. Create Backup\n");
        printf("3. Restore Backup\n");
        printf("4. System Statistics\n");
        printf("0. Back to Main Menu\n");
        printf("Enter choice: ");
        scanf("%d", &choice);
        
        switch (choice) {
            case 1: addUser(); break;
            case 2: createBackup(); break;
            case 3: restoreBackup(); break;
            case 4: 
                printf("\n=== SYSTEM STATISTICS ===\n");
                printf("Active Products: %d\n", productCount);
                printf("Next Product ID: %d\n", nextProductID);
                printf("Next Sale ID: %d\n", nextSaleID);
                printf("Current User: %s (%s)\n", currentUser, isAdmin ? "Admin" : "User");
                break;
            case 0: break;
            default: printf("Invalid choice!\n");
        }
    } while (choice != 0);
}

void mainMenu(void) {
    int choice;
    
    printf("Welcome to Advanced Supermarket Management System!\n");
    printf("==================================================\n");
    printf("User: %s (%s)\n", currentUser, isAdmin ? "Admin" : "User");
    
    do {
        printf("\n=== MAIN MENU ===\n");
        printf("1. Manage Inventory\n");
        printf("2. Process Sales\n");
        printf("3. View Reports\n");
        printf("4. Export Data\n");
        printf("5. Admin Panel\n");
        printf("0. Exit\n");
        printf("Enter choice: ");
        scanf("%d", &choice);
        
        switch (choice) {
            case 1: inventoryMenu(); break;
            case 2: salesMenu(); break;
            case 3: reportsMenu(); break;
            case 4: exportMenu(); break;
            case 5: adminMenu(); break;
            case 0: 
                printf("Thank you for using the Advanced Supermarket System!\n");
                break;
            default: 
                printf("Invalid choice! Please try again.\n");
        }
    } while (choice != 0);
}

// ============================================================================
// MAIN FUNCTION
// ============================================================================

int main(void) {
    // Display program information
    printf("================================================================================\n");
    printf("                   ADVANCED SUPERMARKET MANAGEMENT SYSTEM                      \n");
    printf("                              Version 2.0 - Phase 8                           \n");
    printf("================================================================================\n");
    printf("Features:\n");
    printf("- Complete Inventory Management (CRUD Operations)\n");
    printf("- Sales Processing with Discount System\n");
    printf("- Advanced Reports & Analytics\n");
    printf("- CSV Export Functionality\n");
    printf("- Multi-user Authentication System\n");
    printf("- Backup & Restore Capabilities\n");
    printf("- Real-time Stock Management\n");
    printf("================================================================================\n\n");
    
    // Initialize system components
    printf("Initializing system...\n");
    initializeSystem();
    
    // Authenticate user
    loginSystem();
    
    // Run main menu
    mainMenu();
    
    // Cleanup allocated memory
    if (productCache) {
        free(productCache);
        printf("Product cache cleaned up.\n");
    }
    
    if (indexCache) {
        free(indexCache);
        printf("Index cache cleaned up.\n");
    }
    
    printf("System shutdown complete. Goodbye!\n");
    
    return 0;
}

/* 
 * ============================================================================
 * COMPILATION INSTRUCTIONS
 * ============================================================================
 * 
 * To compile the multi-file project:
 * gcc -o supermarket_advanced sm-main.c sm-functions.c
 * 
 * Alternative compilation with more warnings:
 * gcc -Wall -Wextra -o supermarket_advanced sm-main.c sm-functions.c
 * 
 * To run:
 * ./supermarket_advanced        (Linux/Mac)
 * supermarket_advanced.exe      (Windows)
 * 
 * ============================================================================
 * PROJECT STRUCTURE
 * ============================================================================
 * 
 * sm-header.h     - Contains all includes, defines, structs, and function prototypes
 * sm-functions.c  - Contains all function implementations (includes sm-header.h)
 * sm-main.c       - Contains menu functions and main() (includes sm-header.h)
 * 
 * ============================================================================
 * DEFAULT LOGIN CREDENTIALS
 * ============================================================================
 * 
 * Username: admin
 * Password: admin123
 * Role: Administrator (full access)
 * 
 * ============================================================================
 * FILES CREATED BY THE SYSTEM
 * ============================================================================
 * 
 * Data Files (Binary):
 * - products.dat           (Product inventory database)
 * - sales.dat             (Sales transaction history)
 * - product_index.dat     (Fast lookup index)
 * - users.dat             (User authentication database)
 * 
 * Export Files (CSV):
 * - products_export_YYYYMMDD.csv
 * - sales_export_YYYYMMDD.csv
 * - low_stock_report_YYYYMMDD.csv
 * 
 * Backup Files:
 * - backup_YYYYMMDD_HHMMSS_products.dat
 * - backup_YYYYMMDD_HHMMSS_sales.dat
 * 
 * ============================================================================
 * PHASE 8 FEATURES IMPLEMENTED
 * ============================================================================
 * 
 * ✓ CSV Export Functionality
 *   - Export products with categories and discounts
 *   - Export sales history with detailed transaction data
 *   - Export low stock reports for inventory management
 * 
 * ✓ Password-Protected Admin Access
 *   - Multi-user login system with role-based access
 *   - Secure authentication with attempt limits
 *   - Admin-only functions (user management, backups)
 * 
 * ✓ Backup & Restore Capabilities
 *   - Automatic timestamped backups
 *   - Complete system restore functionality
 *   - Data integrity preservation
 * 
 * ✓ Advanced Filtering & Reporting
 *   - Date range reports with flexible filtering
 *   - Monthly and category-based analytics
 *   - Best-selling products analysis
 *   - Enhanced reports with discount tracking
 * 
 * ✓ Discount & Promotion System
 *   - Per-product discount percentages
 *   - Automatic discount application in sales
 *   - Promotion management and analytics
 * 
 * ✓ Enhanced User Interface
 *   - Professional menu system
 *   - Improved data presentation
 *   - Role-based access control
 *   - Better error handling and validation
 */